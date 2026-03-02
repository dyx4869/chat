# <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">Nginx（TCP 负载） + Redis + Hiredis 部署</font>
## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">一、Nginx 编译安装（支持 TCP 负载）</font>
```plain
sudo apt install -y build-essential libpcre3-dev zlib1g-dev libssl-dev
tar -xvf nginx-1.29.5.tar.gz
cd nginx-1.29.5
./configure --with-stream
make && make install
```



```plain
stream {
    upstream MyServer {
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
    }

    server {
        proxy_connect_timeout 1s;
        # proxy_timeout 3s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">三、Nginx 常用命令</font>
```plain
# 启动
./nginx

# 停止
./nginx -s stop

# 重新加载配置
./nginx -s reload
```

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">四、Redis 编译安装</font>
```plain
tar -zxvf /usr/local/src/redis-7.0.15.tar.gz
cd redis-7.0.15
make && make install
```

## <font style="color:rgb(0, 0, 0);background-color:rgba(0, 0, 0, 0);">五、Hiredis 编译安装</font>
```plain
git clone https://github.com/redis/hiredis
cd hiredis
make
make install
ldconfig /usr/local/lib
```

