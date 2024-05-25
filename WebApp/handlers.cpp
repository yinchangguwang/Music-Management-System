#include "handlers.h"

#include <vector>

#include "rendering.h"

bserv::db_relation_to_object orm_user{
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("username"),
	bserv::make_db_field<std::string>("password"),
	bserv::make_db_field<bool>("is_superuser"),
	bserv::make_db_field<std::string>("first_name"),
	bserv::make_db_field<std::string>("last_name"),
	bserv::make_db_field<std::string>("email"),
	bserv::make_db_field<bool>("is_active")
};

bserv::db_relation_to_object orm_singer{
	bserv::make_db_field<int>("S_"),
	bserv::make_db_field<std::string>("Sname"),
	bserv::make_db_field<std::string>("Spicture")
};

bserv::db_relation_to_object orm_album{
	bserv::make_db_field<int>("A_"),
	bserv::make_db_field<std::string>("Aname"),
	bserv::make_db_field<int>("S_")
};

bserv::db_relation_to_object orm_tag{
	bserv::make_db_field<int>("T_"),
	bserv::make_db_field<std::string>("Tname"),
	bserv::make_db_field<int>("Tsupport")
};

bserv::db_relation_to_object orm_music{
	bserv::make_db_field<int>("M_"),
	bserv::make_db_field<std::string>("Mname"),
	bserv::make_db_field<int>("A_")
};

bserv::db_relation_to_object orm_remark{
	bserv::make_db_field<int>("R_"),
	bserv::make_db_field<std::string>("Rcontext"),
	bserv::make_db_field<int>("Rmark"),
	bserv::make_db_field<int>("id"),
	bserv::make_db_field<std::string>("username"),
	bserv::make_db_field<int>("M_")
};

bserv::db_relation_to_object orm_score{
	bserv::make_db_field<int>("score"),
};

bserv::db_relation_to_object orm_album_management{
	bserv::make_db_field<int>("A_"),
	bserv::make_db_field<std::string>("Aname"),
	bserv::make_db_field<int>("S_"),
	bserv::make_db_field<std::string>("Sname"),
	bserv::make_db_field<std::string>("Spicture")
};

bserv::db_relation_to_object orm_music_management{
	bserv::make_db_field<int>("M_"),
	bserv::make_db_field<std::string>("Mname"),
	bserv::make_db_field<int>("A_"),
	bserv::make_db_field<std::string>("Aname"),
	bserv::make_db_field<int>("S_"),
	bserv::make_db_field<std::string>("Sname"),
	bserv::make_db_field<std::string>("Spicture")
};

bserv::db_relation_to_object orm_tag_management{
	bserv::make_db_field<int>("T_"),
	bserv::make_db_field<std::string>("Tname")
};

std::optional<boost::json::object> get_user(
	bserv::db_transaction& tx,
	const boost::json::string& username) {
	bserv::db_result r = tx.exec(
		"select * from auth_user where username = ?", username);
	lginfo << r.query(); // this is how you log info
	return orm_user.convert_to_optional(r);
}

std::string get_or_empty(
	boost::json::object& obj,
	const std::string& key) {
	return obj.count(key) ? obj[key].as_string().c_str() : "";
}

std::nullopt_t hello(
	bserv::response_type& response,
	std::shared_ptr<bserv::session_type> session_ptr) {
	bserv::session_type& session = *session_ptr;
	boost::json::object obj;
	if (session.count("user")) {
		if (!session.count("count")) {
			session["count"] = 0;
		}
		auto& user = session["user"].as_object();
		session["count"] = session["count"].as_int64() + 1;
		obj = {
			{"welcome", user["username"]},
			{"count", session["count"]}
		};
	}
	else {
		obj = { {"msg", "hello, world!"} };
	}
	response.body() = boost::json::serialize(obj);
	response.prepare_payload();
	return std::nullopt;
}

boost::json::object user_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}
	if (params.count("password") == 0) {
		return {
			{"success", false},
			{"message", "`password` is required"}
		};
	}
	auto username = params["username"].as_string();
	bool is_superuser = atof(params["is_superuser"].as_string().c_str() );
	bserv::db_transaction tx{ conn };
	auto opt_user = get_user(tx, username);
	if (opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "`username` existed"}
		};
	}
	auto password = params["password"].as_string();
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(?, password, is_superuser, "
		"first_name, last_name, email, is_active) values "
		"(?, ?, ?, ?, ?, ?, ?)", bserv::db_name("auth_user"),
		bserv::db_name("username"),
		username,
		bserv::utils::security::encode_password(
			password.c_str()), is_superuser,
		get_or_empty(params, "first_name"),
		get_or_empty(params, "last_name"),
		get_or_empty(params, "email"), true);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "user registered"}
	};
}

boost::json::object add_singer_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int S_ = atof(params["S_"].as_string().c_str());
	auto Sname = params["Sname"].as_string();
	auto Spicture = params["Spicture"].as_string();
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(S_, Sname, Spicture) "
		"values "
		"(?, ?, ?)", bserv::db_name("singer"),
		S_, Sname, Spicture);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "singer added"}
	};
}

boost::json::object add_album_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Aname = params["Aname"].as_string();
	auto Sname = params["Sname"].as_string();
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("insert into ? "
		"(Aname, S_)  "
		"values "
		"(?, "
		"(select S_ from singer where Sname = ?) ); ",
		bserv::db_name("album"), Aname, Sname);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "album added"}
	};
}

boost::json::object add_music_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Mname = params["Mname"].as_string();
	auto Aname = params["Aname"].as_string();
	auto Sname = params["Sname"].as_string();
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("INSERT into music "
		"(Mname, A_) values "
		"(?, "
		"(select album.A_ from album, singer where album.S_ = singer.S_ and album.Aname = ? and singer.Sname = ? )  "
		");", Mname, Aname, Sname );
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "music added"}
	};
}

boost::json::object add_tag_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Tname = params["Tname"].as_string();
	bserv::db_transaction tx{ conn };
	std::cout << "1" << std::endl;
	bserv::db_result r = tx.exec("INSERT into tag (Tname, Tsupport) values (?, 0);", Tname);
	std::cout << "2" << std::endl;
	lginfo << r.query();
	tx.commit();
	std::cout << "3" << std::endl;
	return {
		{"success", true},
		{"message", "tag added"}
	};
}

boost::json::object add_music_tag_register(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Tname = params["Tname"].as_string();
	int M_ = atof(params["M_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("INSERT into tag_belong (T_, M_) values ((select T_ from tag where tag.Tname = ?), ?);", Tname, M_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "music_tag added"}
	};
}

boost::json::object delete_singer_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int S_ = atof(params["S_"].as_string().c_str() );
	bserv::db_transaction tx{ conn };
	bserv::db_result v = tx.exec("delete from remark where M_ in (select M_ from music, album where music.A_ = album.A_ and album.S_ = ?)", S_);
	bserv::db_result u = tx.exec("delete from tag_belong where M_ in (select M_ from music, album where music.A_ = album.A_ and album.S_ = ?)", S_);
	bserv::db_result t = tx.exec("delete from music where A_ in (select A_ from album where album.S_ =?)", S_);
	bserv::db_result s = tx.exec("delete from album where S_ = ?", S_);
	bserv::db_result r = tx.exec("delete from singer where S_ = ?", S_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "singer deleted"}
	};
}

boost::json::object delete_album_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int A_ = atof(params["A_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result u = tx.exec("delete from remark where M_ in (select M_ from music where A_ = ?)", A_);
	bserv::db_result t = tx.exec("delete from tag_belong where M_ in (select M_ from music where A_ = ?)", A_);
	bserv::db_result s = tx.exec("delete from music where A_ = ?", A_);
	bserv::db_result r = tx.exec("delete from album where A_ = ?", A_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "album deleted"}
	};
}

boost::json::object delete_music_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int M_ = atof(params["M_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result t = tx.exec("delete from remark where M_ = ?", M_);
	bserv::db_result s = tx.exec("delete from tag_belong where M_ = ?", M_);
	bserv::db_result r = tx.exec("delete from music where M_ = ?", M_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "music deleted"}
	};
}

boost::json::object delete_tag_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int T_ = atof(params["T_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result s = tx.exec("delete from tag_belong where T_ = ?", T_);
	bserv::db_result r = tx.exec("delete from tag where T_ = ?", T_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "tag deleted"}
	};
}

boost::json::object delete_music_tag_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int T_ = atof(params["T_"].as_string().c_str());
	int M_ = atof(params["M_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("delete from tag_belong where T_ = ? and M_ = ?", T_, M_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "music_tag deleted"}
	};
}

boost::json::object delete_remark_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int R_ = atof(params["R_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("delete from remark where R_ = ?", R_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "remark deleted"}
	};
}

boost::json::object delete_user_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int id = atof(params["id"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result s = tx.exec("delete from remark where id = ?", id);
	bserv::db_result r = tx.exec("delete from auth_user where id = ?", id);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "user deleted"}
	};
}

boost::json::object update_user_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int S_ = atof(params["S_"].as_string().c_str());
	auto Sname = params["Sname"].as_string();
	auto Spicture = params["Spicture"].as_string();
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("update singer set Sname = ?, Spicture = ?  where S_=?;", Sname, Spicture, S_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "user updated"}
	};
}

boost::json::object update_singer_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	int S_ = atof(params["S_"].as_string().c_str());
	auto Sname = params["Sname"].as_string();
	auto Spicture = params["Spicture"].as_string();
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("update singer set Sname = ?, Spicture = ?  where S_=?;", Sname, Spicture, S_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "singer updated"}
	};
}

boost::json::object update_album_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Aname = params["Aname"].as_string();
	auto Sname = params["Sname"].as_string();
	int A_ = atof(params["A_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("update album set Aname = ?, S_ = (select S_ from singer where Sname = ?) where A_ = ?", 
								Aname, Sname, A_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "album updated"}
	};
}

boost::json::object update_music_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Mname = params["Mname"].as_string();
	auto Aname = params["Aname"].as_string();
	auto Sname = params["Sname"].as_string();
	int M_ = atof(params["M_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("update music set Mname = ?, "
						"A_= (select album.A_ from album, singer where album.S_ = singer.S_ and Sname = ? and Aname = ?) where M_ = ? ",
						Mname, Sname, Aname, M_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "music updated"}
	};
}

boost::json::object update_tag_from_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn) {
	auto Tname = params["Tname"].as_string();
	int T_ = atof(params["T_"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec("update tag set Tname = ? where T_ = ?", Tname, T_);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "tag updated"}
	};
}

boost::json::object user_login(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	if (request.method() != boost::beast::http::verb::post) {
		throw bserv::url_not_found_exception{};
	}
	if (params.count("username") == 0) {
		return {
			{"success", false},
			{"message", "`username` is required"}
		};
	}
	if (params.count("password") == 0) {
		return {
			{"success", false},
			{"message", "`password` is required"}
		};
	}
	auto username = params["username"].as_string();
	bserv::db_transaction tx{ conn };
	auto opt_user = get_user(tx, username);
	if (!opt_user.has_value()) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	auto& user = opt_user.value();
	if (!user["is_active"].as_bool()) {
		return {
			{"success", false},
			{"message", "invalid username/password"}
		};
	}
	auto password = params["password"].as_string();
	auto encoded_password = user["password"].as_string();
	if (!bserv::utils::security::check_password(
		password.c_str(), encoded_password.c_str())) {
		return {
			{"success", false},
			{"message", "wrong username/password"}
		};
	}
	bserv::session_type& session = *session_ptr;
	session["user"] = user;
	auto superuser = user;
	bool is_superuser = user["is_superuser"].as_bool();
	std::cout << "is superuser?" << is_superuser << std::endl;
	if (is_superuser == 1)
	{
		std::cout << "set" << std::endl;
		session["superuser"] = superuser;
		std::cout << "is superuser2" << session["superuser"].as_object()["is_superuser"].as_bool() << std::endl;
	}
	return {
		{"success", true},
		{"message", "login successfully"}
	};
}

boost::json::object find_user(
	std::shared_ptr<bserv::db_connection> conn,
	const std::string& username) {
	bserv::db_transaction tx{ conn };
	auto user = get_user(tx, username.c_str());
	if (!user.has_value()) {
		return {
			{"success", false},
			{"message", "requested user does not exist"}
		};
	}
	user.value().erase("id");
	user.value().erase("password");
	return {
		{"success", true},
		{"user", user.value()}
	};
}

boost::json::object user_logout(
	std::shared_ptr<bserv::session_type> session_ptr) {
	bserv::session_type& session = *session_ptr;
	if (session.count("user")) {
		session.erase("user");
	}
	if (session.count("superuser")) {
		session.erase("superuser");
	}
	return {
		{"success", true},
		{"message", "logout successfully"}
	};
}

boost::json::object send_request(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::http_client> client_ptr,
	boost::json::object&& params) {
	auto obj = client_ptr->post_for_value(
		"localhost", "8080", "/echo", { {"request", params} }
	);
	if (session->count("cnt") == 0) {
		(*session)["cnt"] = 0;
	}
	(*session)["cnt"] = (*session)["cnt"].as_int64() + 1;
	return { {"response", obj}, {"cnt", (*session)["cnt"]} };
}

boost::json::object echo(
	boost::json::object&& params) {
	return { {"echo", params} };
}

boost::json::object list_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	const std::string& s_page_id) {
	int page_id = std::stoi(s_page_id);
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from auth_user where is_active;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec(
		"select u.id, u.username, u.password, u.is_superuser, u.first_name, u.last_name, u.email, u.is_active "
		"from auth_user u "
		"where u.is_active "
		"group by u.id, u.username, u.password, u.is_superuser, u.first_name, u.last_name, u.email, u.is_active "
		"limit 10 offset ?;",
		(page_id - 1) * 10);
	lginfo << db_res.query();
	auto users = orm_user.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto& user : users) {
		user.erase("id");
		user.erase("password");
		json_users.push_back(user);
	}
	return { {"users", json_users}, {"total_users", total_users}, {"total_pages", total_pages} };
}

std::nullopt_t ws_echo(
	std::shared_ptr<bserv::session_type> session,
	std::shared_ptr<bserv::websocket_server> ws_server) {
	ws_server->write_json((*session)["cnt"]);
	while (true) {
		try {
			std::string data = ws_server->read();
			ws_server->write(data);
		}
		catch (bserv::websocket_closed&) {
			break;
		}
	}
	return std::nullopt;
}


std::nullopt_t serve_static_files(
	bserv::response_type& response,
	const std::string& path) {
	return serve(response, path);
}


std::nullopt_t index(
	const std::string& template_path,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object& context) {
	bserv::session_type& session = *session_ptr;
	if (session.contains("user")) {
		context["user"] = session["user"];
	}
	if (session.contains("superuser")) {
		context["superuser"] = session["superuser"];
	}
	return render(response, template_path, context);
}

std::nullopt_t index_page(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response) {
	boost::json::object context;
	lgdebug << "view singer: " << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from singer;");
	lginfo << db_res.query();
	std::size_t total_singers = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total singers: " << total_singers << std::endl;
	db_res = tx.exec("select * from singer;");
	lginfo << db_res.query();
	auto singers = orm_singer.convert_to_vector(db_res);
	boost::json::array json_singers;
	for (auto& singer : singers) {
		json_singers.push_back(singer);
	}
	context["singers"] = json_singers;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_login(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	lgdebug << params << std::endl;
	auto context = user_login(request, std::move(params), conn, session_ptr);
	lgdebug << "view singer: " << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from singer;");
	lginfo << db_res.query();
	std::size_t total_singers = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total singers: " << total_singers << std::endl;
	db_res = tx.exec("select * from singer;");
	lginfo << db_res.query();
	auto singers = orm_singer.convert_to_vector(db_res);
	boost::json::array json_singers;
	for (auto& singer : singers) {
		json_singers.push_back(singer);
	}
	context["singers"] = json_singers;
	lginfo << "login: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t form_logout(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response) {
	auto context = user_logout(session_ptr);
	lgdebug << "view singer: " << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from singer;");
	lginfo << db_res.query();
	std::size_t total_singers = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total singers: " << total_singers << std::endl;
	db_res = tx.exec("select * from singer;");
	lginfo << db_res.query();
	auto singers = orm_singer.convert_to_vector(db_res);
	boost::json::array json_singers;
	for (auto& singer : singers) {
		json_singers.push_back(singer);
	}
	context["singers"] = json_singers;
	lginfo << "logout: " << context << std::endl;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from auth_user;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from auth_user limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto users = orm_user.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto& user : users) {
		json_users.push_back(user);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["users"] = json_users;
	return index("users.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_users_login(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view users: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from auth_user;");
	lginfo << db_res.query();
	std::size_t total_users = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total users: " << total_users << std::endl;
	int total_pages = (int)total_users / 10;
	if (total_users % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from auth_user limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto users = orm_user.convert_to_vector(db_res);
	boost::json::array json_users;
	for (auto& user : users) {
		json_users.push_back(user);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["users"] = json_users;

	lgdebug << "view singer: " << std::endl;
	db_res = tx.exec("select count(*) from singer;");
	lginfo << db_res.query();
	std::size_t total_singers = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total singers: " << total_singers << std::endl;
	db_res = tx.exec("select * from singer;");
	lginfo << db_res.query();
	auto singers = orm_singer.convert_to_vector(db_res);
	boost::json::array json_singers;
	for (auto& singer : singers) {
		json_singers.push_back(singer);
	}
	context["singers"] = json_singers;
	return index("index.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_singer(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view singers: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from singer;");
	lginfo << db_res.query();
	std::size_t total_singers = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total singers: " << total_singers << std::endl;
	int total_pages = (int)total_singers / 10;
	if (total_singers % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select * from singer limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto singers = orm_singer.convert_to_vector(db_res);
	boost::json::array json_singers;
	for (auto& singer : singers) {
		json_singers.push_back(singer);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["singers"] = json_singers;
	return index("singer_management.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_album(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view albums: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from album, singer where album.S_=singer.S_;");
	lginfo << db_res.query();
	std::size_t total_albums = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total albums: " << total_albums << std::endl;
	int total_pages = (int)total_albums / 10;
	if (total_albums % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select A_, Aname, album.S_, Sname, Spicture from album, singer where album.S_=singer.S_ limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto albums = orm_album_management.convert_to_vector(db_res);
	boost::json::array json_albums;
	for (auto& album : albums) {
		json_albums.push_back(album);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["albums"] = json_albums;
	return index("album_management.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_music(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view music: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from music, album, singer where music.A_=album.A_ and album.S_=singer.S_ ;");
	lginfo << db_res.query();
	std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total music: " << total_music << std::endl;
	int total_pages = (int)total_music / 10;
	if (total_music % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select M_, Mname, album.A_, Aname, singer.S_, Sname, Spicture from music, album, singer where music.A_=album.A_ and album.S_=singer.S_ limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto music = orm_music_management.convert_to_vector(db_res);
	boost::json::array json_music;
	for (auto& m : music) {
		json_music.push_back(m);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["music"] = json_music;
	return index("music_management.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_music_search(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context,
	std::string Mname_search) {
	lgdebug << "view music: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from music, album, singer where music.A_=album.A_ and album.S_=singer.S_ and Mname like ?;", "%" + Mname_search + "%");
	lginfo << db_res.query();
	std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total music: " << total_music << std::endl;
	int total_pages = (int)total_music / 10;
	if (total_music % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select M_, Mname, album.A_, Aname, singer.S_, Sname, Spicture from music, album, singer where music.A_=album.A_ and album.S_=singer.S_ and music.Mname like ? limit 10 offset ?;", "%" + Mname_search + "%", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto music = orm_music_management.convert_to_vector(db_res);
	boost::json::array json_music;
	for (auto& m : music) {
		json_music.push_back(m);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["music"] = json_music;
	return index("music_management.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_tag(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	boost::json::object&& context) {
	lgdebug << "view tags: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from tag ;");
	lginfo << db_res.query();
	std::size_t total_tags = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total tags: " << total_tags << std::endl;
	int total_pages = (int)total_tags / 10;
	if (total_tags % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select T_, Tname from tag limit 10 offset ?;", (page_id - 1) * 10);
	lginfo << db_res.query();
	auto tags = orm_tag_management.convert_to_vector(db_res);
	boost::json::array json_tags;
	for (auto& tag : tags) {
		json_tags.push_back(tag);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["tags"] = json_tags;
	return index("tag_management.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_music_tag(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	int page_id,
	int music_id,
	boost::json::object&& context) {
	lgdebug << "view music_tags: " << page_id << std::endl;
	bserv::db_transaction tx{ conn };
	bserv::db_result db_res = tx.exec("select count(*) from tag, tag_belong where tag.T_ = tag_belong.T_ and tag_belong.M_ = ? ;", music_id);
	lginfo << db_res.query();
	std::size_t total_music_tags = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total music_tags: " << total_music_tags << std::endl;
	int total_pages = (int)total_music_tags / 10;
	if (total_music_tags % 10 != 0) ++total_pages;
	lgdebug << "total pages: " << total_pages << std::endl;
	db_res = tx.exec("select tag.T_, tag.Tname from tag, tag_belong where tag.T_ = tag_belong.T_ and tag_belong.M_ = ? limit 10 offset ?;", music_id, (page_id - 1) * 10);
	lginfo << db_res.query();
	auto music_tags = orm_tag_management.convert_to_vector(db_res);
	boost::json::array json_music_tags;
	for (auto& music_tag : music_tags) {
		json_music_tags.push_back(music_tag);
	}
	boost::json::object pagination;
	if (total_pages != 0) {
		pagination["total"] = total_pages;
		if (page_id > 1) {
			pagination["previous"] = page_id - 1;
		}
		if (page_id < total_pages) {
			pagination["next"] = page_id + 1;
		}
		int lower = page_id - 3;
		int upper = page_id + 3;
		if (page_id - 3 > 2) {
			pagination["left_ellipsis"] = true;
		}
		else {
			lower = 1;
		}
		if (page_id + 3 < total_pages - 1) {
			pagination["right_ellipsis"] = true;
		}
		else {
			upper = total_pages;
		}
		pagination["current"] = page_id;
		boost::json::array pages_left;
		for (int i = lower; i < page_id; ++i) {
			pages_left.push_back(i);
		}
		pagination["pages_left"] = pages_left;
		boost::json::array pages_right;
		for (int i = page_id + 1; i <= upper; ++i) {
			pages_right.push_back(i);
		}
		pagination["pages_right"] = pages_right;
		context["pagination"] = pagination;
	}
	context["tags"] = json_music_tags;

	db_res = tx.exec("select * from music where M_ = ? ;", music_id);
	lginfo << db_res.query();
	auto music = orm_music.convert_to_vector(db_res);
	boost::json::array json_music;
	for (auto& m : music) {
		json_music.push_back(m);
	}
	context["music"] = json_music;
	return index("music_tag.html", session_ptr, response, context);
}

std::nullopt_t redirect_to_music(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& context,
	int singer_num,
	int album_num,
	int tag_num,
	int music_num) {
	lgdebug << "view music_content: " << std::endl;
	bserv::db_transaction tx{ conn };


	bserv::db_result db_res = tx.exec("select count(*) from music where music.M_ = ?", music_num);
	lginfo << db_res.query();
	std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total music: " << total_music << std::endl;
	db_res = tx.exec("select * from music where music.M_ = ?", music_num);
	lginfo << db_res.query();

	auto music = orm_music.convert_to_vector(db_res);
	boost::json::array json_music;
	for (auto& m : music) {
		json_music.push_back(m);
	}
	context["music"] = json_music;


	db_res = tx.exec("select count(*) from remark, auth_user where remark.M_ = ? and remark.id = auth_user.id", music_num);
	lginfo << db_res.query();
	std::size_t total_remarks = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total remarks: " << total_remarks << std::endl;
	db_res = tx.exec("select R_, Rcontext, Rmark, auth_user.id, username, M_ from remark, auth_user where remark.M_ = ? and remark.id = auth_user.id", music_num);
	lginfo << db_res.query();

	auto remarks = orm_remark.convert_to_vector(db_res);
	boost::json::array json_remarks;
	for (auto& remark : remarks) {
		json_remarks.push_back(remark);
	}
	context["remarks"] = json_remarks;



	db_res = tx.exec("select AVG(Rmark) from remark where remark.M_ = ?", music_num);
	lginfo << db_res.query();
	db_res = tx.exec("select floor(AVG(Rmark)) from remark where remark.M_ = ?", music_num);
	lginfo << db_res.query();
	if (total_remarks != 0)
	{
		auto score = orm_score.convert_to_vector(db_res);
		std::cout << "..." << std::endl;
		boost::json::array json_score;
		for (auto& score_single : score) {
			json_score.push_back(score_single);
		}
		context["score"] = json_score;
	}
	else
	{
		std::vector<boost::json::object> score;
		std::cout << "..." << std::endl;
		boost::json::array json_score;
		for (auto& score_single : score) {
			json_score.push_back(score_single);
		}
		context["score"] = json_score;
	}
	

	db_res = tx.exec("select count(*) from tag, tag_belong where tag_belong.M_ = ? and tag.T_ = tag_belong.T_", music_num);
	lginfo << db_res.query();
	std::size_t total_tags = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total tags: " << total_tags << std::endl;
	db_res = tx.exec("select * from tag, tag_belong where tag_belong.M_ = ? and tag.T_ = tag_belong.T_", music_num);
	lginfo << db_res.query();

	if (total_tags != 0)
	{
		auto tags = orm_tag.convert_to_vector(db_res);
		boost::json::array json_tags;
		for (auto& tag : tags) {
			json_tags.push_back(tag);
		}
		context["tags"] = json_tags;
	}
	else
	{
		std::vector<boost::json::object>  tags;
		boost::json::array json_tags;
		for (auto& tag : tags) {
			json_tags.push_back(tag);
		}
		context["tags"] = json_tags;
	}

	return index("music_content.html", session_ptr, response, context);
}


std::nullopt_t redirect_to_singer_index(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	boost::json::object&& context,
	int singer_num,
	int album_num, 
	int tag_num,
	std::string music_search) {
	lgdebug << "view singer: " << std::endl;
	bserv::db_transaction tx{ conn };


	bserv::db_result db_res = tx.exec("SELECT count(*) from album where album.S_= ?;", singer_num);
	lginfo << db_res.query();
	std::size_t total_albums = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total albums: " << total_albums << std::endl;
	db_res = tx.exec("SELECT * from album where album.S_= ?;", singer_num);
	lginfo << db_res.query();
	auto albums = orm_album.convert_to_vector(db_res);
	boost::json::array json_albums;
	for (auto& album : albums) {
		json_albums.push_back(album);
	}
	context["albums"] = json_albums;


	db_res = tx.exec("SELECT count(*) from tag where exists( select * from tag_belong, music, album where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ?);", singer_num);
	lginfo << db_res.query();
	std::size_t total_tags = (*db_res.begin())[0].as<std::size_t>();
	lgdebug << "total tags: " << total_tags << std::endl;
	db_res = tx.exec( "SELECT * from tag where exists( select * from tag_belong, music, album where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ?);", singer_num);
	lginfo << db_res.query();
	auto tags = orm_tag.convert_to_vector(db_res);
	boost::json::array json_tags;
	for (auto& tag : tags) {
		json_tags.push_back(tag);
	}
	context["tags"] = json_tags;

 
	if (album_num != 0)	{
		if (tag_num != 0) {
			db_res = tx.exec("SELECT count(*) from music where exists( select * from tag_belong, album, tag where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ? and tag.T_ = ? and album.A_ = ? and music.Mname like ? );", singer_num, tag_num, album_num, "%" + music_search + "%");
			lginfo << db_res.query();
			std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
			lgdebug << "total music: " << total_music << std::endl;
			db_res = tx.exec("SELECT * from music where exists( select * from tag_belong, album, tag where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ? and tag.T_ = ? and album.A_ = ? and music.Mname like ?);", singer_num, tag_num, album_num, "%" + music_search + "%");
			lginfo << db_res.query();
		}
		else {
			db_res = tx.exec("SELECT count(*) from music where exists( select * from album where music.A_ = album.A_ and album.S_ = ? and album.A_ = ? and music.Mname like ?);", singer_num, album_num, music_search + "%");
			lginfo << db_res.query();
			std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
			lgdebug << "total music: " << total_music << std::endl;
			db_res = tx.exec("SELECT * from music where exists( select * from album where music.A_ = album.A_ and album.S_ = ? and album.A_ = ? and music.Mname like ?);", singer_num, album_num, music_search + "%");
			lginfo << db_res.query();
		}
	}
	else {
		if (tag_num != 0) {
			db_res = tx.exec("SELECT count(*) from music where exists( select * from tag_belong, album, tag where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ? and tag.T_ = ? and music.Mname like ?);", singer_num, tag_num, "%" + music_search + "%");
			lginfo << db_res.query();
			std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
			lgdebug << "total music: " << total_music << std::endl;
			db_res = tx.exec("SELECT * from music where exists( select * from tag_belong, album, tag where tag_belong.T_ = tag.T_ and tag_belong.M_ = music.M_ and music.A_ = album.A_ and album.S_ = ? and tag.T_ = ? and music.Mname like ?);", singer_num, tag_num, "%" + music_search + "%");
			lginfo << db_res.query();
		}
		else {
			db_res = tx.exec("SELECT count(*) from music where exists( select * from album where music.A_ = album.A_ and album.S_ = ? and music.Mname like ?);", singer_num, "%" + music_search + "%");
			lginfo << db_res.query();
			std::size_t total_music = (*db_res.begin())[0].as<std::size_t>();
			lgdebug << "total music: " << total_music << std::endl;
			db_res = tx.exec("SELECT * from music where exists( select * from album where music.A_ = album.A_ and album.S_ = ? and music.Mname like ?);", singer_num, "%" + music_search + "%");
			lginfo << db_res.query();
		}
	}

	auto music = orm_music.convert_to_vector(db_res);
	boost::json::array json_music;
	for (auto& m : music) {
		json_music.push_back(m);
	}
	context["music"] = json_music;
	return index("music.html", session_ptr, response, context);
}

std::nullopt_t view_users(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_users(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t singer_management(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_singer(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t album_management(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_album(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t music_management(
	std::shared_ptr<bserv::db_connection> conn,
	boost::json::object&& params,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;

	boost::json::object&& params_tmp = std::move(params);
	std::string M_tmp;
	if (!params_tmp["Mname_search"].is_string())
		M_tmp = "";
	else
		M_tmp = params_tmp["Mname_search"].as_string().c_str();

	return redirect_to_music_search(conn, session_ptr, response, page_id, std::move(context), M_tmp);
}

std::nullopt_t tag_management(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	boost::json::object context;
	return redirect_to_tag(conn, session_ptr, response, page_id, std::move(context));
}

std::nullopt_t music_tag(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& music_num,
	const std::string& page_num) {
	int page_id = std::stoi(page_num);
	int music_id = std::stoi(music_num);
	boost::json::object context;
	return redirect_to_music_tag(conn, session_ptr, response, page_id, music_id, std::move(context));
}

std::nullopt_t form_add_user(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = user_register(request, std::move(params), conn);
	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_singer(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = add_singer_register(request, std::move(params), conn);
	return redirect_to_singer(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_album(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = add_album_register(request, std::move(params), conn);
	return redirect_to_album(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_music(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = add_music_register(request, std::move(params), conn);
	return redirect_to_music(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_tag(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = add_tag_register(request, std::move(params), conn);
	return redirect_to_tag(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_music_tag(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = add_music_tag_register(request, std::move(params), conn);
	boost::json::object&& params_tmp = std::move(params);
	int M_tmp = atof(params_tmp["M_"].as_string().c_str());
	return redirect_to_music_tag(conn, session_ptr, response, 1, M_tmp, std::move(context));
}

std::nullopt_t delete_singer(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_singer_from_database(request, std::move(params), conn);
	return redirect_to_singer(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_album(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_album_from_database(request, std::move(params), conn);
	return redirect_to_album(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_music(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_music_from_database(request, std::move(params), conn);
	std::cout << "first step done" << std::endl;
	return redirect_to_music(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_tag(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_tag_from_database(request, std::move(params), conn);
	return redirect_to_tag(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_music_tag(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_music_tag_from_database(request, std::move(params), conn);
	boost::json::object&& params_tmp = std::move(params);
	int M_tmp = atof(params_tmp["M_"].as_string().c_str());
	return redirect_to_music_tag(conn, session_ptr, response, 1, M_tmp, std::move(context));
}

std::nullopt_t delete_remark(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_remark_from_database(request, std::move(params), conn);
	return redirect_to_music(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t delete_user(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = delete_user_from_database(request, std::move(params), conn);
	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
}

// std::nullopt_t update_user(
// 	bserv::request_type& request,
// 	bserv::response_type& response,
// 	boost::json::object&& params,
// 	std::shared_ptr<bserv::db_connection> conn,
// 	std::shared_ptr<bserv::session_type> session_ptr) {
// 	boost::json::object context = update_user_from_database(request, std::move(params), conn);
// 	return redirect_to_users(conn, session_ptr, response, 1, std::move(context));
// }

std::nullopt_t update_singer(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = update_singer_from_database(request, std::move(params), conn);
	return redirect_to_singer(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t update_album(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = update_album_from_database(request, std::move(params), conn);
	return redirect_to_album(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t update_music(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = update_music_from_database(request, std::move(params), conn);
	return redirect_to_music(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t update_tag(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr) {
	boost::json::object context = update_tag_from_database(request, std::move(params), conn);
	return redirect_to_tag(conn, session_ptr, response, 1, std::move(context));
}

std::nullopt_t form_add_remark(
	bserv::request_type& request,
	bserv::response_type& response,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	const std::string& singer_num,
	const std::string& album_num,
	const std::string& tag_num,
	const std::string& music_num) {
	int singer_id = std::stoi(singer_num);
	int album_id = std::stoi(album_num);
	int tag_id = std::stoi(tag_num);
	int music_id = std::stoi(music_num);

	bserv::session_type& session = *session_ptr;
	auto id = session["user"].as_object()["id"].as_int64();
	

	boost::json::object context = add_remark_to_database(request, std::move(params), conn, id, music_id);
	return music_content(conn, session_ptr, response, singer_num, album_num, tag_num, music_num);
}

boost::json::object add_remark_to_database(
	bserv::request_type& request,
	boost::json::object&& params,
	std::shared_ptr<bserv::db_connection> conn,
	int id,
	int music_id) {
	auto Rcontext = params["Rcontext"].as_string();
	int Rmark = atof(params["Rmark"].as_string().c_str());
	bserv::db_transaction tx{ conn };
	bserv::db_result r = tx.exec(
		"insert into ? "
		"(Rcontext, Rmark, id, M_) "
		"values "
		"(?, ?, ?, ?)", bserv::db_name("remark"),
		Rcontext,
		Rmark,
		id,
		music_id);
	lginfo << r.query();
	tx.commit();
	return {
		{"success", true},
		{"message", "remark added"}
	};
}

std::nullopt_t singer_index(
	std::shared_ptr<bserv::db_connection> conn,
	boost::json::object&& params,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& singer_num,
	const std::string& album_num, 
	const std::string& tag_num) {
	int singer_id = std::stoi(singer_num);
	int album_id = std::stoi(album_num);
	int tag_id = std::stoi(tag_num);
	boost::json::object context;

	boost::json::object&& params_tmp = std::move(params);
	std::string M_tmp;
	if (!params_tmp["Mname_search"].is_string())
		M_tmp = "";
	else
		M_tmp = params_tmp["Mname_search"].as_string().c_str();
	return redirect_to_singer_index(conn, session_ptr, response, std::move(context), singer_id, album_id, tag_id, M_tmp);
}

std::nullopt_t music_content(
	std::shared_ptr<bserv::db_connection> conn,
	std::shared_ptr<bserv::session_type> session_ptr,
	bserv::response_type& response,
	const std::string& singer_num,
	const std::string& album_num,
	const std::string& tag_num,
	const std::string& music_num) {
	int singer_id = std::stoi(singer_num);
	int album_id = std::stoi(album_num);
	int tag_id = std::stoi(tag_num);
	int music_id = std::stoi(music_num);
	boost::json::object context;
	std::cout << "enter" << std::endl;
	return redirect_to_music(conn, session_ptr, response, std::move(context), singer_id, album_id, tag_id, music_id);
}