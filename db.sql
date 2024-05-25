CREATE TABLE auth_user (
    id serial PRIMARY KEY,
    username character varying(255) NOT NULL UNIQUE,
    password character varying(255) NOT NULL,
    is_superuser boolean NOT NULL,
    first_name character varying(255) NOT NULL,
    last_name character varying(255) NOT NULL,
    email character varying(255) NOT NULL,
    is_active boolean NOT NULL
);

CREATE TABLE singer (
    S_ serial PRIMARY KEY,
    Sname character varying(255) NOT NULL UNIQUE,
    Spicture character varying(255)
);

CREATE TABLE album (
    A_ serial PRIMARY KEY,
    Aname character varying(255) NOT NULL,
    S_ integer,
    FOREIGN KEY(S_) REFERENCES singer(S_)
);

CREATE TABLE music (
    M_ serial PRIMARY KEY,
    Mname character varying(255) NOT NULL,
    A_ integer,
    FOREIGN KEY(A_) REFERENCES album(A_)
);

CREATE TABLE remark(
    R_ serial PRIMARY KEY,
    Rcontext character varying(255),
    Rmark INTEGER,
    id integer,
    M_ integer,
    FOREIGN KEY(id) REFERENCES auth_user (id),
    FOREIGN KEY(M_) REFERENCES music(M_)
);

CREATE TABLE tag(
    T_ serial PRIMARY KEY,
    Tname character varying(255),
    Tsupport integer
);

CREATE TABLE tag_belong(
    T_ integer,
    M_ integer,
    PRIMARY KEY(T_, M_),
    FOREIGN KEY(T_) REFERENCES tag(T_),
    FOREIGN KEY(M_) REFERENCES music(M_)
);

INSERT INTO singer(Sname, Spicture)
VALUES
    ('汪苏泷', 'wangsulong'),
    ('林俊杰', 'linjunjie'),
    ('薛之谦', 'xuezhiqian'),
    ('G.E.M.邓紫棋', 'G.E.M.dengziqi'),
    ('张杰', 'zhangjie'),
    ('单依纯', 'shanyichun'),
    ('Taylor Swift', 'Taylor Swift'),
    ('毛不易', 'maobuyi'),
    ('陈奕迅', 'chenyixun'),
    ('李荣浩', 'lironghao');
