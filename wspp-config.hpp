/**
 * This example is presently used as a scratch space. It may or may not be broken
 * at any given time.
 */

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

template <typename T>
struct wsperf_config : public T {
    // pull default settings from our core config
    typedef T core;

    typedef core::concurrency_type concurrency_type;
    typedef core::request_type request_type;
    typedef core::response_type response_type;
    //typedef core::message_type message_type;
    //typedef core::con_msg_manager_type con_msg_manager_type;
    //typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;
    typedef websocketpp::message_buffer::fixed::policy::message message_type;
    typedef websocketpp::message_buffer::fixed::policy::con_msg_manager con_msg_manager_type;
    //typedef websocketpp::message_buffer::fixed::policy::con_msg_manager endpoint_msg_manager_type;
    typedef websocketpp::message_buffer::fixed::policy::endpoint_msg_manager endpoint_msg_manager_type;
    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;
    typedef core::rng_type rng_type;
    typedef core::endpoint_base endpoint_base;

    static bool const enable_multithreading = true;

    struct transport_config : public core::transport_config {
        typedef core::concurrency_type concurrency_type;
        typedef core::elog_type elog_type;
        typedef core::alog_type alog_type;
        typedef core::request_type request_type;
        typedef core::response_type response_type;

        static bool const enable_multithreading = true;
    };

    typedef websocketpp::transport::asio::endpoint<transport_config>
        transport_type;

    static const websocketpp::log::level elog_level =
        websocketpp::log::elevel::none;
    static const websocketpp::log::level alog_level =
        websocketpp::log::alevel::none;
};

typedef websocketpp::client<wsperf_config<websocketpp::config::asio_client>> client;
typedef websocketpp::client<wsperf_config<websocketpp::config::asio_tls_client>> client_tls;

// convenience typedefs
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
