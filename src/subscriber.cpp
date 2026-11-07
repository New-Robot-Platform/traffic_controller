
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const std::string host = "localhost";
const std::string port = "1883";

const std::string DFLT_ADDRESS{"tcp://" + host + ":" + port};

const string TOPIC = "hello";
const string CLIENT_ID = "test_client_id";
const int QOS = 2;

class action_listener : public virtual mqtt::iaction_listener
{
    std::string name;

    void on_failure(const mqtt::token& tok) override
    {
        cout << name << " failure";
        if (tok.get_message_id() != 0)
            cout << "for token : [" << tok.get_message_id() << "]" << std::endl;
        cout << std::endl;
    }

    void on_success(const mqtt::token& tok) override
    {
        if (name == "Subscription") std::ofstream("/tmp/mqtt-subscribed") << "ready\n";
        cout << name << " success";
        if (tok.get_message_id() != 0)
            cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty()) cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

   public:
    action_listener(const std::string& name) : name(name)
    {
    }
};

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    int retry = 0;
    mqtt::async_client& cli;
    mqtt::connect_options& connOpts;

    action_listener subListener;

    void reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));

        try
        {
            cli.connect(connOpts, nullptr, *this);
        }
        catch (const mqtt::exception& exc)
        {
            std::cerr << "Error: " << exc.what() << std::endl;
            exit(1);
        }
    }

    void on_failure(const mqtt::token& tok) override
    {
        std::cout << "Connection Attempt failed " << std::endl;
        if (++retry > 5 /*N_RETRY_ATTEMPTS*/) exit(1);
        reconnect();
    }

    void on_success(const mqtt::token& tok) override
    {
    }

    void connected(const std::string& cause) override
    {
        std::cout << "\nConnecttion success" << std::endl;
        std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
                  << "\t for client " << CLIENT_ID << " using QoS" << QOS << "\n"
                  << "\nPress Q<Enter> to Quit\n"
                  << std::endl;

        cli.subscribe(TOPIC, QOS, nullptr, subListener);
    }

    void connection_lost(const string& cause) override
    {
        std::remove("/tmp/mqtt-subscribed");
        cout << "\nConnection Lost" << std::endl;
        if (!cause.empty()) std::cout << "\tcause: " << cause << std::endl;

        std::cout << "Reconnection ... " << std::endl;
        retry = 0;
        reconnect();
    }

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        cout << "Message arrived" << std::endl;
        cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
        cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
    }
    void delivery_complete(mqtt::delivery_token_ptr token) override
    {
    }

   public:
    callback(mqtt::async_client& client, mqtt::connect_options& options)
        : cli(client), connOpts(options), subListener(action_listener("Subscription"))
    {
    }
};

int main(int argc, char** argv)
{
    std::remove("/tmp/mqtt-subscribed");
    const char* configuredURI = std::getenv("MQTT_BROKER_URI");
    const std::string address = configuredURI ? configuredURI : DFLT_ADDRESS;
    mqtt::async_client client(address, "client_robot");

    mqtt::connect_options conOps = mqtt::connect_options_builder()
                                       .user_name("user")
                                       .password("passwd")
                                       .keep_alive_interval(seconds(30))
                                       .clean_session(false)
                                       .finalize();

    callback cb(client, conOps);
    client.set_callback(cb);

    try
    {
        cout << "Connection to MQTT broker at '" << address << "'..." << endl;
        client.connect(conOps, nullptr, cb);
    }
    catch (mqtt::exception& e)
    {
        cerr << "MQTT conntection failed  cause : " << e.what() << endl;
        return 1;
    }

    while (std::tolower(cin.get()) != 'q');

    try
    {
        cout << "\n Disconnection from the mqtt server ..." << flush;
        client.disconnect()->wait();
        cout << "OK" << endl;
    }
    catch (const mqtt::exception& exc)
    {
        cerr << exc << endl;
        return 1;
    }

    return 0;
}
