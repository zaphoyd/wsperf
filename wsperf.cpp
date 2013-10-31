/**
 * This example is presently used as a scratch space. It may or may not be broken
 * at any given time.
 */

#include "wspp-config.hpp"

#include <iostream>
#include <chrono>

struct open_handshake_stats {
    std::chrono::high_resolution_clock::time_point s_start;
    std::chrono::high_resolution_clock::time_point s_tcp_pre_init;
    std::chrono::high_resolution_clock::time_point s_tcp_post_init;
    std::chrono::high_resolution_clock::time_point s_open;
    std::chrono::high_resolution_clock::time_point s_close;

    // some sort of status
    bool s_fail = false;
};

template <typename client_type>
class handshake_test {
public:
    typedef handshake_test<client_type> type;
    typedef std::chrono::duration<int,std::micro> dur_type;

    typedef typename client_type::connection_ptr connection_ptr;

    handshake_test () {
        // silence access/error messages
        m_endpoint.set_access_channels(websocketpp::log::alevel::none);
        m_endpoint.set_error_channels(websocketpp::log::elevel::none);

        // Initialize ASIO
        m_endpoint.init_asio();

        // Register our handlers
        if (m_endpoint.is_secure()) {
            //m_endpoint.set_tls_init_handler(bind(&type::on_tls_init,this,::_1));
        }

        m_endpoint.set_tcp_pre_init_handler(bind(&type::on_tcp_pre_init,this,::_1));
        m_endpoint.set_tcp_post_init_handler(bind(&type::on_tcp_post_init,this,::_1));
        m_endpoint.set_open_handler(bind(&type::on_open,this,::_1));
        m_endpoint.set_close_handler(bind(&type::on_close,this,::_1));
    }

    void start(std::string uri) {
        // TODO: how many connections to start with?

        launch_connection(uri);


        m_endpoint.run();
    }


    void launch_connection(std::string uri) {
        websocketpp::lib::error_code ec;
        connection_ptr con = m_endpoint.get_connection(uri, ec);

        if (ec) {
        	m_endpoint.get_alog().write(websocketpp::log::alevel::app,ec.message());
        }

        m_endpoint.connect(con);
	    con->s_start = std::chrono::high_resolution_clock::now();
    }

    void on_tcp_pre_init(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_tcp_pre_init = std::chrono::high_resolution_clock::now();
    }
    void on_tcp_post_init(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_tcp_post_init = std::chrono::high_resolution_clock::now();
    }

    context_ptr on_tls_init(websocketpp::connection_hdl hdl) {
        context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::single_dh_use);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
    }

    void on_open(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_open = std::chrono::high_resolution_clock::now();
        con->s_fail = false;
        con->close(websocketpp::close::status::going_away,"");
    }
    
    void on_fail(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_open = std::chrono::high_resolution_clock::now();
        con->s_fail = true;
    }
    
    void on_close(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_close = std::chrono::high_resolution_clock::now();

        // TODO: get lock
        m_stats_list.push_back(*websocketpp::lib::static_pointer_cast<open_handshake_stats>(con));
        test_complete();
    }

    void test_complete() {
        std::cout << "[";
        for (auto i : m_stats_list) {
            std::cout << "{\"tcp_pre_init\":" 
                      << std::chrono::duration_cast<dur_type>(i.s_tcp_pre_init-i.s_start).count() 
            std::cout << ",\"tcp_post_init\":" 
                      << std::chrono::duration_cast<dur_type>(i.s_tcp_post_init-i.s_tcp_pre_init).count() 
            std::cout << ",\"open\":" 
                      << std::chrono::duration_cast<dur_type>(i.s_open-i.s_tcp_post_init).count() 
            std::cout << ",\"close\":" 
                      << std::chrono::duration_cast<dur_type>(i.s_close-i.s_open).count() 
                      << "}";
        }
        std::cout << "]";
    }
private:
    client_type m_endpoint;

    std::vector<open_handshake_stats> m_stats_list;
};


int main(int argc, char* argv[]) {
	std::string uri = "wss://echo.websocket.org";

	if (argc == 2) {
	    uri = argv[1];
	}

	try {
        if (uri.substr(0,3) == "wss") {
            //handshake_test<client_tls<open_handshake_stats>> endpoint;
            //endpoint.start(uri);
            std::cout << "wss not supported at the moment" << std::endl;
        } else {
            handshake_test<client<open_handshake_stats>> endpoint;
            endpoint.start(uri);
        }
    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
