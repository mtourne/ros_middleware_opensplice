#include <iostream>
#include <stdexcept>

#include <ccpp_dds_dcps.h>

#include "rosidl_generator_cpp/MessageTypeSupport.h"
#include "ros_middleware_interface/handles.h"
#include "rosidl_typesupport_opensplice_cpp/MessageTypeSupport.h"

namespace ros_middleware_interface
{

const char * _prismtech_opensplice_identifier = "opensplice_static";

void init()
{
    std::cout << "init()" << std::endl;
    std::cout << "  init() get_instance" << std::endl;
    DDS::DomainParticipantFactory_var dpf_ = DDS::DomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  init() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };
}

ros_middleware_interface::NodeHandle create_node()
{
    std::cout << "create_node()" << std::endl;

    std::cout << "  create_node() " << _prismtech_opensplice_identifier << std::endl;

    std::cout << "  create_node() get_instance" << std::endl;
    DDS::DomainParticipantFactory_var dpf_ = DDS::DomainParticipantFactory::get_instance();
    if (!dpf_) {
        printf("  create_node() could not get participant factory\n");
        throw std::runtime_error("could not get participant factory");
    };

    DDS::DomainId_t domain = 0;

    std::cout << "  create_node() create_participant in domain " << domain  << std::endl;
    DDS::DomainParticipant * participant = dpf_->create_participant(
        domain, PARTICIPANT_QOS_DEFAULT, NULL,
        DDS::STATUS_MASK_NONE);
    if (!participant) {
        printf("  create_node() could not create participant\n");
        throw std::runtime_error("could not create participant");
    };

    std::cout << "  create_node() pass opaque node handle" << std::endl;

    ros_middleware_interface::NodeHandle node_handle = {
        _prismtech_opensplice_identifier,
        participant
    };
    return node_handle;
}

struct CustomPublisherInfo {
  DDS::DataWriter * topic_writer_;
  ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks_;
};

ros_middleware_interface::PublisherHandle create_publisher(const ros_middleware_interface::NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
    std::cout << "create_publisher()" << std::endl;

    if (node_handle._implementation_identifier != _prismtech_opensplice_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }

    std::cout << "create_publisher() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_publisher() extract participant from opaque node handle" << std::endl;
    DDS::DomainParticipant * participant = (DDS::DomainParticipant *)node_handle._data;

    ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = (ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks*)type_support_handle._data;
    std::string type_name = std::string(callbacks->_package_name) + "::dds_::" + callbacks->_message_name + "_";


    std::cout << "  create_publisher() invoke register callback" << std::endl;
    callbacks->_register_type(participant, type_name.c_str());


    DDS::PublisherQos publisher_qos;
    DDS::ReturnCode_t status = participant->get_default_publisher_qos(publisher_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_publisher_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default publisher qos failed");
    };

    std::cout << "  create_publisher() create dds publisher" << std::endl;
    DDS::Publisher * dds_publisher = participant->create_publisher(
        publisher_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!dds_publisher) {
        printf("  create_publisher() could not create publisher\n");
        throw std::runtime_error("could not create publisher");
    };


    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_topic_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default topic qos failed");
    };

    std::cout << "  create_publisher() create topic: " << topic_name << std::endl;
    if (std::string(topic_name).find("/") != std::string::npos)
    {
      throw std::runtime_error("topic_name contains a '/'");
    }
    DDS::Topic * topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
    );
    if (!topic) {
        printf("  create_topic() could not create topic\n");
        throw std::runtime_error("could not create topic");
    };


    DDS::DataWriterQos default_datawriter_qos;
    status = dds_publisher->get_default_datawriter_qos(default_datawriter_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_datawriter_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datawriter qos failed");
    };

    std::cout << "  create_publisher() create data writer" << std::endl;
    DDS::DataWriter * topic_writer = dds_publisher->create_datawriter(
        topic, default_datawriter_qos,
        NULL, DDS::STATUS_MASK_NONE);


    std::cout << "  create_publisher() build opaque publisher handle" << std::endl;
    CustomPublisherInfo* custom_publisher_info = new CustomPublisherInfo();
    custom_publisher_info->topic_writer_ = topic_writer;
    custom_publisher_info->callbacks_ = callbacks;

    ros_middleware_interface::PublisherHandle publisher_handle = {
        _prismtech_opensplice_identifier,
        custom_publisher_info
    };
    return publisher_handle;
}

void publish(const ros_middleware_interface::PublisherHandle& publisher_handle, const void * ros_message)
{
    //std::cout << "publish()" << std::endl;

    if (publisher_handle._implementation_identifier != _prismtech_opensplice_identifier)
    {
        printf("publisher handle not from this implementation\n");
        printf("but from: %s\n", publisher_handle._implementation_identifier);
        throw std::runtime_error("publisher handle not from this implementation");
    }

    //std::cout << "  publish() extract data writer and type code from opaque publisher handle" << std::endl;
    CustomPublisherInfo * custom_publisher_info = (CustomPublisherInfo*)publisher_handle._data;
    DDS::DataWriter * topic_writer = custom_publisher_info->topic_writer_;
    const ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = custom_publisher_info->callbacks_;


    //std::cout << "  publish() invoke publish callback" << std::endl;
    callbacks->_publish(topic_writer, ros_message);
}

struct CustomSubscriberInfo {
  DDS::DataReader * topic_reader_;
  ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks_;
};

ros_middleware_interface::SubscriberHandle create_subscriber(const NodeHandle& node_handle, const rosidl_generator_cpp::MessageTypeSupportHandle & type_support_handle, const char * topic_name)
{
  std::cout << "create_subscriber()" << std::endl;

    if (node_handle._implementation_identifier != _prismtech_opensplice_identifier)
    {
        printf("node handle not from this implementation\n");
        printf("but from: %s\n", node_handle._implementation_identifier);
        throw std::runtime_error("node handle not from this implementation");
    }
    std::cout << "create_subscriber() " << node_handle._implementation_identifier << std::endl;

    std::cout << "  create_subscriber() extract participant from opaque node handle" << std::endl;
    DDS::DomainParticipant * participant = (DDS::DomainParticipant *)node_handle._data;



    ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = (ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks*)type_support_handle._data;
    std::string type_name = std::string(callbacks->_package_name) + "::dds_::" + callbacks->_message_name + "_";


    std::cout << "  create_subscriber() invoke register callback" << std::endl;
    callbacks->_register_type(participant, type_name.c_str());


    DDS::SubscriberQos subscriber_qos;
    DDS::ReturnCode_t status = participant->get_default_subscriber_qos(subscriber_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_subscriber_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default subscriber qos failed");
    };

    std::cout << "  create_subscriber() create dds subscriber" << std::endl;
    DDS::Subscriber * dds_subscriber = participant->create_subscriber(
        subscriber_qos, NULL, DDS::STATUS_MASK_NONE);
    if (!dds_subscriber) {
        printf("  create_subscriber() could not create subscriber\n");
        throw std::runtime_error("could not create subscriber");
    };


    DDS::TopicQos default_topic_qos;
    status = participant->get_default_topic_qos(default_topic_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_topic_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default topic qos failed");
    };

    std::cout << "  create_subscriber() create topic" << std::endl;
    DDS::Topic * topic = participant->create_topic(
        topic_name, type_name.c_str(), default_topic_qos, NULL,
        DDS::STATUS_MASK_NONE
    );
    if (!topic) {
        printf("  create_topic() could not create topic\n");
        throw std::runtime_error("could not create topic");
    };


    DDS::DataReaderQos default_datareader_qos;
    status = dds_subscriber->get_default_datareader_qos(default_datareader_qos);
    if (status != DDS::RETCODE_OK) {
        printf("get_default_datareader_qos() failed. Status = %d\n", status);
        throw std::runtime_error("get default datareader qos failed");
    };

    std::cout << "  create_subscriber() create data reader" << std::endl;
    DDS::DataReader * topic_reader = dds_subscriber->create_datareader(
        topic, default_datareader_qos,
        NULL, DDS::STATUS_MASK_NONE);

    std::cout << "  topic reader" << topic_reader << std::endl;

    std::cout << "  create_subscriber() build opaque subscriber handle" << std::endl;
    CustomSubscriberInfo* custom_subscriber_info = new CustomSubscriberInfo();
    custom_subscriber_info->topic_reader_ = topic_reader;
    custom_subscriber_info->callbacks_ = callbacks;

    ros_middleware_interface::SubscriberHandle subscriber_handle = {
        _prismtech_opensplice_identifier,
        custom_subscriber_info
    };
    return subscriber_handle;

}

bool take(const ros_middleware_interface::SubscriberHandle& subscriber_handle, void * ros_message)
{
  if (subscriber_handle.implementation_identifier_ != _prismtech_opensplice_identifier)
  {
    printf("subscriber handle not from this implementation\n");
    printf("but from: %s\n", subscriber_handle.implementation_identifier_);
    throw std::runtime_error("subscriber handle not from this implementation");
  }

  //std::cout << "  take() extract data writer and type code from opaque subscriber handle" << std::endl;
  CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)subscriber_handle.data_;
  DDS::DataReader * topic_reader = custom_subscriber_info->topic_reader_;
  const ros_middleware_opensplice_cpp::MessageTypeSupportCallbacks * callbacks = custom_subscriber_info->callbacks_;


  return callbacks->_take(topic_reader, ros_message);
}

ros_middleware_interface::GuardConditionHandle create_guard_condition()
{
  ros_middleware_interface::GuardConditionHandle guard_condition_handle;
  guard_condition_handle.implementation_identifier_ = _prismtech_opensplice_identifier;
  guard_condition_handle.data_ = new DDS::GuardCondition();
  return guard_condition_handle;
}

void trigger_guard_condition(const ros_middleware_interface::GuardConditionHandle& guard_condition_handle)
{
  if (guard_condition_handle.implementation_identifier_ != _prismtech_opensplice_identifier)
  {
    printf("guard condition handle not from this implementation\n");
    printf("but from: %s\n", guard_condition_handle.implementation_identifier_);
    throw std::runtime_error("guard condition handle not from this implementation");
  }

  DDS::GuardCondition * guard_condition = (DDS::GuardCondition*)guard_condition_handle.data_;
  guard_condition->set_trigger_value(true);
}

void wait(ros_middleware_interface::SubscriberHandles& subscriber_handles, ros_middleware_interface::GuardConditionHandles& guard_condition_handles, bool non_blocking)
{
  DDS::WaitSet waitset;

  // add a condition for each subscriber
  for (unsigned long i = 0; i < subscriber_handles.subscriber_count_; ++i)
  {
    void * data = subscriber_handles.subscribers_[i];
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
    DDS::DataReader * topic_reader = custom_subscriber_info->topic_reader_;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);
    waitset.attach_condition(condition);
  }

  // add a condition for each guard condition
  for (unsigned long i = 0; i < guard_condition_handles.guard_condition_count_; ++i)
  {
    void * data = guard_condition_handles.guard_conditions_[i];
    DDS::GuardCondition * guard_condition = (DDS::GuardCondition*)data;
    waitset.attach_condition(guard_condition);
  }

  // invoke wait until one of the conditions triggers
  DDS::ConditionSeq active_conditions;
  DDS::Duration_t timeout;
  timeout.sec = non_blocking ? 0 : 1;
  timeout.nanosec = 0;
  DDS::ReturnCode_t status = DDS::RETCODE_TIMEOUT;
  while (DDS::RETCODE_TIMEOUT == status)
  {
    status = waitset.wait(active_conditions, timeout);
    if (DDS::RETCODE_TIMEOUT == status) {
      if (non_blocking)
      {
        break;
      }
      continue;
    };
    if (status != DDS::RETCODE_OK) {
      printf("wait() failed. Status = %d\n", status);
      throw std::runtime_error("wait failed");
    };
  }

  // set subscriber handles to zero for all not triggered status conditions
  for (unsigned long i = 0; i < subscriber_handles.subscriber_count_; ++i)
  {
    void * data = subscriber_handles.subscribers_[i];
    CustomSubscriberInfo * custom_subscriber_info = (CustomSubscriberInfo*)data;
    DDS::DataReader* topic_reader = custom_subscriber_info->topic_reader_;
    DDS::StatusCondition * condition = topic_reader->get_statuscondition();
    if (!condition->get_trigger_value())
    {
      // if the status condition was not triggered
      // reset the subscriber handle
      subscriber_handles.subscribers_[i] = 0;
    }
  }

  // set guard condition handles to zero for all not triggered guard conditions
  for (unsigned long i = 0; i < guard_condition_handles.guard_condition_count_; ++i)
  {
    void * data = guard_condition_handles.guard_conditions_[i];
    DDS::GuardCondition * guard_condition = (DDS::GuardCondition*)data;
    if (!guard_condition->get_trigger_value())
    {
      // if the guard condition was not triggered
      // reset the guard condition handle
      guard_condition_handles.guard_conditions_[i] = 0;
    }
    else
    {
      // reset the trigger value
      guard_condition->set_trigger_value(false);
    }
  }
}

}
