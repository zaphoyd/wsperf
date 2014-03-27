/**
 * This example is presently used as a scratch space. It may or may not be broken
 * at any given time.
 */

#define _WEBSOCKETPP_CPP11_STL_

#include "wspp-config.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <mutex>
#include <thread>

struct message_data {
    message_data(size_t size, std::chrono::high_resolution_clock::time_point start)
      : s_size(size)
      , s_start(start) {}
    
    size_t s_size;
    std::chrono::high_resolution_clock::time_point s_start;
    std::chrono::high_resolution_clock::time_point s_end;
};

struct open_handshake_stats {
    size_t s_id;
    
    std::chrono::high_resolution_clock::time_point s_start;
    std::chrono::high_resolution_clock::time_point s_tcp_pre_init;
    std::chrono::high_resolution_clock::time_point s_tcp_post_init;
    std::chrono::high_resolution_clock::time_point s_open;
    std::chrono::high_resolution_clock::time_point s_close;

    // some sort of status
    bool s_fail;
    
    // list of message data for this connection
    std::string s_message;
    std::vector<message_data> s_message_list;
    size_t s_message_total;
    size_t s_message_in_flight;
};

void on_socket_init(websocketpp::connection_hdl hdl, boost::asio::ip::tcp::socket & s) {
    boost::asio::ip::tcp::no_delay option(true);
    s.set_option(option);
}

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

        m_endpoint.set_socket_init_handler(bind(&on_socket_init,::_1,::_2));

        m_endpoint.set_tcp_pre_init_handler(bind(&type::on_tcp_pre_init,this,::_1));
        m_endpoint.set_tcp_post_init_handler(bind(&type::on_tcp_post_init,this,::_1));
        m_endpoint.set_open_handler(bind(&type::on_open,this,::_1));
        m_endpoint.set_fail_handler(bind(&type::on_fail,this,::_1));
        m_endpoint.set_close_handler(bind(&type::on_close,this,::_1));
        m_endpoint.set_message_handler(bind(&type::on_message,this,::_1,::_2));
    }

    void start(std::string uri, size_t num_threads, size_t num_cons, size_t num_parallel_handshakes_low, size_t num_parallel_handshakes_high, std::string logfile) {
        m_stats_list.reserve(num_cons);
        m_uri = uri;
        m_logfile = logfile;
        m_connection_count = num_cons;
        m_max_handshakes_low = num_parallel_handshakes_low;
        m_max_handshakes_high = num_parallel_handshakes_high;
        m_cur_handshakes = 0;
        m_total_connections = 0;
        m_close_immediately = false;
        m_cur_connections = 0;

        m_low_water_mark_count = 0;
        m_high_water_mark_count = 0;

        m_message_size = 8;
        m_message_count = 40000;
        m_message_low = 20;
        m_message_high = 40;

        m_test_start = std::chrono::high_resolution_clock::now();
        m_test_start_wallclock = std::chrono::system_clock::now();

        launch_more_connections();

        if (num_threads > 0) {
            std::vector<std::thread> ts;
            for (size_t i = 0; i < num_threads; i++) {
                ts.push_back(std::thread(&client_type::run, &m_endpoint));
            }
            for (auto & t : ts) {
                t.join();
            }
        } else {
            m_endpoint.run();
        }
    }

    void launch_more_connections() {
        if (m_total_connections < m_connection_count) {
            m_low_water_mark_count++;
        }
        while (m_total_connections < m_connection_count) {
            if (m_cur_handshakes == m_max_handshakes_high) {
                m_high_water_mark_count++;
                break;
            }
            launch_connection(m_uri);
            m_cur_handshakes++;
            m_total_connections++;
        }
    }

    void launch_connection(std::string uri) {
        websocketpp::lib::error_code ec;
        connection_ptr con = m_endpoint.get_connection(uri, ec);

        if (ec) {
        	m_endpoint.get_alog().write(websocketpp::log::alevel::app,ec.message());
        }

        m_endpoint.connect(con);
        con->s_id = m_total_connections;
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

    void send_more_messages(connection_ptr con) {
        /*std::cout << "send_more_messages called " << std::endl;
        std::cout << "(" << con->s_message_total 
                  << "/" << m_message_count << ") with (" << con->s_message_in_flight 
                  << "/" << m_message_high << ") in flight on connection " 
                  << con->s_id << std::endl;*/
        while (con->s_message_total < m_message_count && con->s_message_in_flight < m_message_high) {
            std::copy(
                reinterpret_cast<char *>(&con->s_message_total),
                reinterpret_cast<char *>(&con->s_message_total)+sizeof(con->s_message_total),
                const_cast<char *>(con->s_message.data()));
            
            con->s_message_total++;
            con->s_message_in_flight++;
            con->s_message_list.push_back(message_data(m_message_size,std::chrono::high_resolution_clock::now()));
            std::error_code ec;
            m_endpoint.send(con->get_handle(), con->s_message, websocketpp::frame::opcode::binary,ec);
            if (ec) {
                std::cout << "error sending message: " << ec.message() << std::endl;
                break;
            }
        }
    }
    
    void on_message(websocketpp::connection_hdl hdl, typename client_type::message_ptr msg) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        
        //std::cout << websocketpp::utility::to_hex(msg->get_payload()) << std::endl;
        
        con->s_message_in_flight--;
        
        size_t id = 0;
        
        /*std::copy(
            msg->get_payload().data(),
            msg->get_payload().data()+sizeof(id),
            &id);*/
        id = *reinterpret_cast<size_t*>(const_cast<char *>(msg->get_payload().data()));

        /*std::cout << msg->get_payload().data() << std::endl;
        std::cout << msg->get_payload().data()+sizeof(id) << std::endl;
        std::cout << &id << std::endl;
        std::cout << &con << std::endl;
        std::cout << sizeof(id) << std::endl;
        std::cout << "got message " << id;
        std::cout << " on connection " << con->s_id;
        std::cout << " list size: " << con->s_message_list.size() << std::endl;*/
                
        con->s_message_list[id].s_end = std::chrono::high_resolution_clock::now();
        
        //std::cout << "checking messages in flight: (" << con->s_message_in_flight << "/" << m_message_low << ")" << std::endl;
        
        if (con->s_message_in_flight < m_message_low) {
            send_more_messages(con);
        }
        
        if (con->s_message_total == m_message_count && con->s_message_in_flight == 0) {
            con->close(websocketpp::close::status::going_away,"");
        }
    }

    void on_open(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_open = std::chrono::high_resolution_clock::now();
        con->s_fail = false;

        if (m_close_immediately) {
            con->close(websocketpp::close::status::going_away,"");
        } else if (m_message_count > 0) {
            con->s_message_total = 0;
            con->s_message_in_flight = 0;
            con->s_message.clear();
            con->s_message.append(m_message_size, '*');

            // start sending messages
            send_more_messages(con);
        }

        std::lock_guard<std::mutex> guard(m_stats_lock);
        m_cur_handshakes--;
        m_cur_connections++;

        // check if we need to launch more connections
        if (m_cur_handshakes < m_max_handshakes_low) {
            launch_more_connections();
        }

        // check if we need to close any connections
        if (m_total_connections == m_connection_count) {
            // close connections
        }
    }

    void on_fail(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_open = std::chrono::high_resolution_clock::now();
        con->s_close = con->s_open;
        con->s_fail = true;

        std::lock_guard<std::mutex> guard(m_stats_lock);
        m_cur_handshakes--;

        // Add stats to the list
        m_stats_list.push_back(*websocketpp::lib::static_pointer_cast<open_handshake_stats>(con));

        // check if we need to launch more connections
        if (m_cur_handshakes < m_max_handshakes_low) {
            launch_more_connections();
        }

        // check if we are done
        if (m_stats_list.size() == m_connection_count) {
            test_complete();
        }


    }

    void on_close(websocketpp::connection_hdl hdl) {
        connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        con->s_close = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> guard(m_stats_lock);
        m_cur_connections--;

        // Add stats to the list
        m_stats_list.push_back(*websocketpp::lib::static_pointer_cast<open_handshake_stats>(con));

        // Check it we are done
        if (m_stats_list.size() == m_connection_count) {
            test_complete();
        }
    }

    void test_complete() {
        m_test_end = std::chrono::high_resolution_clock::now();
        m_test_end_wallclock = std::chrono::system_clock::now();

        std::ofstream logfile;
        logfile.open(m_logfile);

        logfile << "{\"total_duration\":"
                << std::chrono::duration_cast<dur_type>(m_test_end-m_test_start).count()
                << ",\"started\":" << m_test_start_wallclock.time_since_epoch().count()
                << ",\"ended\":" << m_test_end_wallclock.time_since_epoch().count()
                << ",\"handshake_throttle_count\":" << m_high_water_mark_count
                << ",\"handshake_resume_count\":" << m_low_water_mark_count
                << ",\"connection_stats\":[";
        bool first = true;
        for (auto i : m_stats_list) {
            logfile << (!first ? "," : "") << "{\"tcp_pre_init\":"
                    << std::chrono::duration_cast<dur_type>(i.s_tcp_pre_init-i.s_start).count()
                    << ",\"tcp_post_init\":"
                    << std::chrono::duration_cast<dur_type>(i.s_tcp_post_init-i.s_tcp_pre_init).count()
                    << ",\"open\":"
                    << std::chrono::duration_cast<dur_type>(i.s_open-i.s_tcp_post_init).count()
                    << ",\"close\":"
                    << std::chrono::duration_cast<dur_type>(i.s_close-i.s_open).count()
                    << ",\"failed\":" << (i.s_fail ? "true" : "false")
                    << "}";
            first = false;
        }
        logfile << "],\"message_stats\":[";
        
        first = true;
        for (auto i : m_stats_list) {
            for (auto j : i.s_message_list) {
                logfile << (!first ? "," : "") << "{\"con\":" 
                        << i.s_id
                        << ",\"size\":"
                        << j.s_size
                        << ",\"rtt\":"
                        << std::chrono::duration_cast<dur_type>(j.s_end-j.s_start).count()
                        << "}";
            first = false;
            }
        }
        
        logfile << "]}" << std::endl;

        logfile.close();
    }
private:
    client_type m_endpoint;

    std::string m_uri;
    std::string m_logfile;
    size_t m_connection_count;
    size_t m_max_handshakes_high;
    size_t m_max_handshakes_low;
    size_t m_cur_handshakes;
    size_t m_cur_connections;
    size_t m_total_connections;

    size_t m_high_water_mark_count;
    size_t m_low_water_mark_count;

    bool m_close_immediately;

    std::chrono::high_resolution_clock::time_point m_test_start;
    std::chrono::high_resolution_clock::time_point m_test_end;

    std::chrono::system_clock::time_point m_test_start_wallclock;
    std::chrono::system_clock::time_point m_test_end_wallclock;

    std::mutex m_stats_lock;
    std::vector<open_handshake_stats> m_stats_list;
    
    size_t m_message_size;
    size_t m_message_count;
    size_t m_message_low;
    size_t m_message_high;
};





typedef websocketpp::client<wsperf_config<websocketpp::config::asio_client, open_handshake_stats>> client_tls;

int main(int argc, char* argv[]) {
    std::string logfile;
	std::string uri;
    size_t num_threads;
    size_t num_cons;
    size_t max_parallel_handshakes_low;
    size_t max_parallel_handshakes_high;

	if (argc == 7) {
	    uri = argv[1];
	    num_threads = atoi(argv[2]);
	    num_cons = atoi(argv[3]);
	    max_parallel_handshakes_low = atoi(argv[4]);
	    max_parallel_handshakes_high = atoi(argv[5]);
        logfile = argv[6];
	} else {
	    std::cout << "Usage: wsperf serverurl num_threads num_connections max_parallel_handshakes_low max_parallel_handshakes_high" << std::endl;
	    std::cout << "Example: wsperf ws://localhost:9002 4 50 25 50 result.json" << std::endl;
	    return 1;
	}

	// some input sanity checking
	if (max_parallel_handshakes_low == 0 || max_parallel_handshakes_low > max_parallel_handshakes_high) {
	    std::cout << "max_parallel_handshakes_low must be positive and less than max_parallel_handshakes_high" << std::endl;
	    return 1;
	}

	try {
        if (uri.substr(0,3) == "wss") {
            //handshake_test<client_tls<open_handshake_stats>> endpoint;
            //endpoint.start(uri);
            std::cout << "wss not supported at the moment" << std::endl;
        } else {
            handshake_test<client_tls> endpoint;
            endpoint.start(uri,num_threads,num_cons,max_parallel_handshakes_low,max_parallel_handshakes_high,logfile);
        }
    } catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
