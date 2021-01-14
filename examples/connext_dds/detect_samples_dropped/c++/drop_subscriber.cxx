/*
* (c) Copyright, Real-Time Innovations, 2021.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "drop.h"
#include "dropSupport.h"
#include "ndds/ndds_cpp.h"
#include "application.h"

using namespace application;

static int shutdown_participant(
    DDSDomainParticipant *participant,
    const char *shutdown_message,
    int status);

// Process data. Returns number of samples processed.
unsigned int process_data(dropDataReader *typed_reader)
{
    dropSeq data_seq;     // Sequence of received data
    DDS_SampleInfoSeq info_seq; // Metadata associated with samples in data_seq
    unsigned int samples_read = 0;

    // Take available data from DataReader's queue
    typed_reader->take(
        data_seq,
        info_seq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);

    // Iterate over all available data
    for (int i = 0; i < data_seq.length(); ++i) {
        // Check if a sample is an instance lifecycle event
        if (info_seq[i].valid_data) {
            // Print data
            std::cout << "Received data" << std::endl;
            dropTypeSupport::print_data(&data_seq[i]);
            samples_read++;
        } else {  // This is an instance lifecycle event with no data payload.
            std::cout << "Received instance state notification" << std::endl;
        }
    }
    // Data loaned from Connext for performance. Return loan when done.
    DDS_ReturnCode_t retcode = typed_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "return loan error " << retcode << std::endl;
    }

    return samples_read;
}

int run_subscriber_application(unsigned int domain_id, unsigned int sample_count)
{
    // Start communicating in a domain, usually one participant per application
    DDSDomainParticipant *participant =
    DDSTheParticipantFactory->create_participant(
        domain_id,
        DDS_PARTICIPANT_QOS_DEFAULT,
        NULL, // listener
        DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        return shutdown_participant(participant, "create_participant error", EXIT_FAILURE);
    }

    // A Subscriber allows an application to create one or more DataReaders
    DDSSubscriber *subscriber = participant->create_subscriber(
        DDS_SUBSCRIBER_QOS_DEFAULT,
        NULL, // listener
        DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        return shutdown_participant(participant, "create_subscriber error", EXIT_FAILURE);
    }

    // Register the datatype to use when creating the Topic
    const char *type_name = dropTypeSupport::get_type_name();
    DDS_ReturnCode_t retcode =
    dropTypeSupport::register_type(participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        return shutdown_participant(participant, "register_type error", EXIT_FAILURE);
    }

    // Create a Topic with a name and a datatype
    DDSTopic *topic = participant->create_topic(
        "Example drop",
        type_name,
        DDS_TOPIC_QOS_DEFAULT,
        NULL, // listener
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        return shutdown_participant(participant, "create_topic error", EXIT_FAILURE);
    }

    DDS_StringSeq parameters(0);
    parameters.length(0);
    DDSContentFilteredTopic *cft = participant->create_contentfilteredtopic(
            "Example drop CFT",
            topic,
            "x < 4",
            parameters);
    if (cft == NULL) {
        return shutdown_participant(participant, "create_contentfilteredtopic error", EXIT_FAILURE);
    }

    DDS_DataReaderQos reader_qos;
    retcode = subscriber->get_default_datareader_qos(reader_qos);
    if (retcode != DDS_RETCODE_OK) {
        return shutdown_participant(participant, "get_default_datareader_qos error", EXIT_FAILURE);
    }
    reader_qos.ownership.kind = DDS_EXCLUSIVE_OWNERSHIP_QOS;

    // This DataReader reads data on "Example drop" Topic
    DDSDataReader *untyped_reader = subscriber->create_datareader(
        cft,
        reader_qos,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (untyped_reader == NULL) {
        return shutdown_participant(participant, "create_datareader error", EXIT_FAILURE);
    }

    // Narrow casts from a untyped DataReader to a reader of your type
    dropDataReader *typed_reader =
    dropDataReader::narrow(untyped_reader);
    if (typed_reader == NULL) {
        return shutdown_participant(participant, "DataReader narrow error", EXIT_FAILURE);
    }

    // Create ReadCondition that triggers when unread data in reader's queue
    DDSReadCondition *read_condition = typed_reader->create_readcondition(
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    if (read_condition == NULL) {
        return shutdown_participant(participant, "create_readcondition error", EXIT_FAILURE);
    }

    // WaitSet will be woken when the attached condition is triggered
    DDSWaitSet waitset;
    retcode = waitset.attach_condition(read_condition);
    if (retcode != DDS_RETCODE_OK) {
        return shutdown_participant(participant, "attach_condition error", EXIT_FAILURE);
    }

    // Main loop. Wait for data to arrive, and process when it arrives
    unsigned int samples_read = 0;
    DDS_DataReaderCacheStatus status;
    while (!shutdown_requested && samples_read < sample_count) {
        DDSConditionSeq active_conditions_seq;

        // Wait for data and report if it does not arrive in 1 second
        DDS_Duration_t wait_timeout = { 1, 0 };
        retcode = waitset.wait(active_conditions_seq, wait_timeout);

        if (retcode == DDS_RETCODE_OK) {
            // If the read condition is triggered, process data
            samples_read += process_data(typed_reader);
        } else {
            if (retcode == DDS_RETCODE_TIMEOUT) {
                std::cout << "No data after 1 second" << std::endl;
            }
        }

        untyped_reader->get_datareader_cache_status(status);
        std::cout << "Samples dropped:"
                  << "\n\t ownership_dropped_sample_count "
                  << status.ownership_dropped_sample_count
                  << "\n\t content_filter_dropped_sample_count "
                  << status.content_filter_dropped_sample_count << std::endl;
    }

    // Cleanup
    return shutdown_participant(participant, "Shutting down", 0);
}

// Delete all entities
static int shutdown_participant(
    DDSDomainParticipant *participant,
    const char *shutdown_message,
    int status)
{
    DDS_ReturnCode_t retcode;

    std::cout << shutdown_message << std::endl;

    if (participant != NULL) {
        // Cleanup everything created by this Participant
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "delete_contained_entities error" << retcode
            << std::endl;
            status = EXIT_FAILURE;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "delete_participant error" << retcode << std::endl;
            status = EXIT_FAILURE;
        }
    }
    return status;
}

int main(int argc, char *argv[])
{

    // Parse arguments and handle control-C
    ApplicationArguments arguments;
    parse_arguments(arguments, argc, argv);
    if (arguments.parse_result == PARSE_RETURN_EXIT) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == PARSE_RETURN_FAILURE) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    NDDSConfigLogger::get_instance()->set_verbosity(arguments.verbosity);

    int status = run_subscriber_application(arguments.domain_id, arguments.sample_count);

    // Releases the memory used by the participant factory.  Optional at
    // application exit
    DDS_ReturnCode_t retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "finalize_instance error" << retcode << std::endl;
        status = EXIT_FAILURE;
    }

    return status;
}
