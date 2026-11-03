#include <atomic>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const string DFLT_SERVER_URI{"mqtt://localhost:1883"};
const string CLIENT_ID{"paho_cpp_async_publish"};

const string PERSIST_DIR{"./persist"};

const string TOPIC{"hello"};

const string PAYLOAD1{"Hello world!"};
const string PAYLOAD2{"Hi There!"};
const string PAYLOAD3{"Is anyone listening?"};
const string PAYLOAD4{"Someone is always listening"};

const string LWT_PAYLOAD{"Last will and testament"};

const int QOS = 1;
const auto TIMEOUT = std::chrono::seconds(10);

class callback : public virtual mqtt::callback
{
   public:
    void connection_lost(const string& cause) override
    {
        cout << "\nConnetion lost" << endl;
        if (!cause.empty()) cout << "\tcause: " << cause << endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr tok) override
    {
        cout << "\tDelivery complete for token : " << (tok ? tok->get_message_id() : -1) << endl;
    }
};

class action_listener : public virtual mqtt::iaction_listener
{
   protected:
    void on_failure(const mqtt::token& tok) override
    {
        cout << "\tListener failure for token: " << tok.get_message_id() << endl;
    }

    void on_success(const mqtt::token& tok) override
    {
        cout << "\tListener success for token: " << tok.get_message_id() << endl;
    }
};

class delivery_action_listener : public action_listener
{
    atomic<bool> done{false};

   public:
    bool is_done() const
    {
        return done.load();
    }

   private:
    void on_failure(const mqtt::token& tok) override
    {
        action_listener::on_failure(tok);
        done = true;
    }

    void on_success(const mqtt::token& tok) override
    {
        action_listener::on_success(tok);
        done = true;
    }
};

int main()
{
    const char* configuredURI = std::getenv("MQTT_BROKER_URI");
    string serverURI = configuredURI ? configuredURI : DFLT_SERVER_URI;
    string clientID = CLIENT_ID;
    mqtt::async_client client(serverURI, clientID, PERSIST_DIR);

    callback cb;
    client.set_callback(cb);

    auto connOpts = mqtt::connect_options_builder()
                        .clean_session()
                        .will(mqtt::message(TOPIC, LWT_PAYLOAD, QOS, false))
                        .connect_timeout(5s)
                        .finalize();

    cout << "....OK..." << endl;

    try
    {
        cout << "\nConnecting .. " << endl;
        mqtt::token_ptr conntok = client.connect(connOpts);
        cout << "Waiting for the connection..." << endl;
        conntok->wait();
        cout << " ...OK" << endl;

        cout << "\nSending message..." << endl;
        mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, PAYLOAD1);
        pubmsg->set_qos(QOS);
        client.publish(pubmsg)->wait_for(TIMEOUT);
        cout << " ...OK" << endl;

        cout << "\nSending Next message " << endl;
        mqtt::delivery_token_ptr pubtok;
        pubtok = client.publish(TOPIC, PAYLOAD2, QOS, false);
        cout << "   ...with token: " << pubtok->get_message_id() << endl;
        cout << "   ...for message with " << pubtok->get_message()->get_payload().size() << " bytes"
             << endl;
        pubtok->wait_for(TIMEOUT);
        cout << " ...OK" << endl;

        cout << "\nSending next message..." << endl;
        action_listener listener;
        pubmsg = mqtt::make_message(TOPIC, PAYLOAD3);
        pubtok = client.publish(pubmsg, nullptr, listener);
        pubtok->wait();
        cout << "  ...OK" << endl;

        cout << "\nSending final message..." << endl;
        delivery_action_listener deliveryListener;
        pubmsg = mqtt::make_message(TOPIC, PAYLOAD4);
        client.publish(pubmsg, nullptr, deliveryListener);

        while (!deliveryListener.is_done())
        {
            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        cout << "OK" << endl;

        auto toks = client.get_pending_delivery_tokens();

        if (!toks.empty()) cout << "Error: There are pending delivery tokens!" << endl;

        cout << "\nDisconnecting..." << endl;
        client.disconnect()->wait();
        cout << "  ...OK" << endl;
    }
    catch (mqtt::exception& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
