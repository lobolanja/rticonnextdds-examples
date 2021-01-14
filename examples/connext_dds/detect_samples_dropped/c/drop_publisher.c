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

/* drop_publisher.c

A publication of data of type drop

This file is derived from code automatically generated by the rtiddsgen
command:

rtiddsgen -language C -example <arch> drop.idl

Example publication of type drop automatically generated by
'rtiddsgen'. To test it, follow these steps:

(1) Compile this file and the example subscription.

(2) Start the subscription on the same domain used for RTI Connext

(3) Start the publication on the same domain used for RTI Connext

(4) [Optional] Specify the list of discovery initial peers and
multicast receive addresses via an environment variable or a file
(in the current working directory) called NDDS_DISCOVERY_PEERS.

You can run any number of publisher and subscriber programs, and can
add and remove them dynamically from the domain.
*/

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_c.h"
#include "drop.h"
#include "dropSupport.h"

/* Delete all entities */
static int publisher_shutdown(
        DDS_DomainParticipant *participant,
        struct DDS_DataWriterQos *writer_qos)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
            DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    retcode = DDS_DataWriterQos_finalize(writer_qos);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize Writer qos error %d\n", retcode);
        status = -1;
    }

    /* RTI Data Distribution Service provides finalize_instance() method on
    domain participant factory for people who want to release memory used
    by the participant factory. Uncomment the following block of code for
    clean destruction of the singleton. */
    /*
    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }
    */

    return status;
}

int publisher_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Publisher *publisher = NULL;
    DDS_Topic *topic = NULL;
    DDS_DataWriter *writer1 = NULL, *writer2 = NULL;
    struct DDS_DataWriterQos writer_qos = DDS_DataWriterQos_INITIALIZER;
    dropDataWriter *drop_writer1 = NULL, *drop_writer2 = NULL;
    drop *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;
    struct DDS_Duration_t send_period = {4,0};

    /* To customize participant QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant(
        DDS_TheParticipantFactory, domainId, &DDS_PARTICIPANT_QOS_DEFAULT,
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* To customize publisher QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    publisher = DDS_DomainParticipant_create_publisher(
        participant, &DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "create_publisher error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* Register type before creating topic */
    type_name = dropTypeSupport_get_type_name();
    retcode = dropTypeSupport_register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* To customize topic QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    topic = DDS_DomainParticipant_create_topic(
        participant, "Example drop",
        type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* Set writer qos */
    retcode = DDS_Publisher_get_default_datawriter_qos(publisher, &writer_qos);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_default_datawriter_qos error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }
    /* Use batching in order to evaluate the CFT in the reader side */
    writer_qos.batch.enable = DDS_BOOLEAN_TRUE;
    writer_qos.batch.max_samples = 1;
    writer_qos.ownership.kind = DDS_EXCLUSIVE_OWNERSHIP_QOS;
    writer_qos.ownership_strength.value = 1;

    /* To customize data writer QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    writer1 = DDS_Publisher_create_datawriter(
            publisher, topic,
            &writer_qos,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (writer1 == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }
    drop_writer1 = dropDataWriter_narrow(writer1);
    if (drop_writer1 == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    writer_qos.ownership_strength.value = 2;
    /* To customize data writer QoS, use
    the configuration file USER_QOS_PROFILES.xml */
    writer2 = DDS_Publisher_create_datawriter(
            publisher,
            topic,
            &writer_qos,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (writer2 == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }
    drop_writer2 = dropDataWriter_narrow(writer2);
    if (drop_writer2 == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* Create data sample for writing */
    instance = dropTypeSupport_create_data_ex(DDS_BOOLEAN_TRUE);
    if (instance == NULL) {
        fprintf(stderr, "dropTypeSupport_create_data error\n");
        publisher_shutdown(participant, &writer_qos);
        return -1;
    }

    /* For a data type that has a key, if the same instance is going to be
    written multiple times, initialize the key here
    and register the keyed instance prior to writing */
    /*
    instance_handle = dropDataWriter_register_instance(
        drop_writer, instance);
    */
    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {

        printf("Writing drop, count %d\n", count);

        /* Modify the data to be written here */
        instance->x = count;
        /* Write data */
        retcode = dropDataWriter_write(
            drop_writer1, instance, &instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "write error %d\n", retcode);
        }
        /* Write data */
        retcode = dropDataWriter_write(
            drop_writer2, instance, &instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "write error %d\n", retcode);
        }

        NDDS_Utility_sleep(&send_period);
    }

    /*
    retcode = dropDataWriter_unregister_instance(
        drop_writer, instance, &instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "unregister instance error %d\n", retcode);
    }
    */
    /* Delete data sample */
    retcode = dropTypeSupport_delete_data_ex(instance, DDS_BOOLEAN_TRUE);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "dropTypeSupport_delete_data error %d\n", retcode);
    }
    /* Cleanup and delete delete all entities */
    return publisher_shutdown(participant, &writer_qos);
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API,
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return publisher_main(domain_id, sample_count);
}

