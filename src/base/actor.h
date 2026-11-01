#pragma once

#include <asio.hpp>
#include <memory>
#include <utility>

class ActorBase : public std::enable_shared_from_this<ActorBase>
{
   public:
    explicit ActorBase(asio::io_context& ioc) : strand_(asio::make_strand(ioc))
    {
    }

    virtual ~ActorBase() = default;

    ActorBase(const ActorBase&) = delete;
    ActorBase& operator=(const ActorBase&) = delete;

    auto get_strand()
    {
        return strand_;
    }

   protected:
    asio::strand<asio::io_context::executor_type> strand_;
};

template <typename MessageType>
class Actor : public ActorBase
{
   public:
    using ActorBase::ActorBase;

    void send(MessageType msg)
    {
        asio::post(strand_,
                   [self = std::static_pointer_cast<Actor<MessageType>>(this->shared_from_this()),
                    message = std::move(msg)]() mutable { self->on_receive(message); });
    }

   protected:
    virtual void on_receive(const MessageType& msg) = 0;
};
