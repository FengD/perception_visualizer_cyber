#pragma once

#include <google/protobuf/message_lite.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <glog/logging.h>

namespace crdc {
namespace airi {

class SubscriptionBase {
 public:
  virtual ~SubscriptionBase() {}

 public:
  virtual void onMessage(const std::string &channel, const void *data, const size_t size) = 0;
};

template <typename MessageT, typename std::enable_if<std::is_base_of<
                                 google::protobuf::MessageLite, MessageT>::value>::type * = nullptr>
class Subscription : public SubscriptionBase {
 public:
  Subscription(
      const std::function<void(const std::string &, const std::shared_ptr<MessageT> &)> &callback)
      : callback_(callback) {}

 public:
  void onMessage(const std::string &channel, const void *data, const size_t size) override {
    auto msg = std::make_shared<MessageT>();
    if(!msg->ParseFromArray(data, size)) {
      LOG(ERROR) << "Failed to decode message to type " << msg->GetTypeName();
      return;
    }
    callback_(channel, msg);
  }

 protected:
  const std::function<void(const std::string &, const std::shared_ptr<MessageT> &)> callback_;
};

}  // namespace airi
}  // namespace crdc
