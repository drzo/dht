/*
 * testmaster.{C,h} --- provides functionality to instruct slaves to do RPCs.
 *
 * Copyright (C) 2002  Thomer M. Gil (thomer@lcs.mit.edu)
 *   		       Massachusetts Institute of Technology
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "async.h"
#include "dhash.h"
#include "test.h"
#include "dhashclient_test.h"

#define TESLA_CONTROL_PORT 8002


class testslave { public:
  testslave() {}
  testslave(str n, int d, int c) : name(n), dhash_port(d), control_port(c) {}
  ~testslave() {}

  str name;
  int dhash_port;
  int control_port;
};


typedef struct {
  unsigned id;
  str p2psocket;
  testslave *s;
  int unixsocket_fd;
  callback<void>::ref cb;
} conthunk;



class testmaster { public:
  // testmaster with filename that contains hostname/port/port.  see
  // config.txt.sample.  most convenient if generated by testwrapper.pl
  testmaster(char *filename);

  // alternative to filename.  dhp -> dhash port numbers,
  // tcp -> tesla control ports.  you shouldn't need this.
  testmaster(vec<str> *names, vec<int> *dhp, vec<int> *tcp);
  ~testmaster();

  // full API available after using setup
  void setup(callback<void>::ref);

  // limited API (see dhashclient_test.h) available after using dry_setup
  // don't use this.  it's for block.C's use only.
  void dry_setup(callback<void>::ref);

  // returns dhash client for certain identifier
  dhashclient_test* dhash(const unsigned id) { return _clients[id]->dhc; }
  dhashclient_test* operator[](const unsigned id) { return dhash(id); }


private:
  unsigned _nhosts;
  bool _busy;

  void unixdomainsock(int i, str &p2psocket, int &fd);
  void addnode(conthunk tx);
  void addnode_cb(conthunk tx, const int there_fd);
  void addnode_cb2(conthunk tx, const int there_fd, ptr<hostent> h, int err);
  void accept_connection(const int unixsocket_fd, const int there_fd, bool dry);
  void pipe(const int, const int);

  void dry_setup_cb(int i, str p2psocket, callback<void>::ref cb, ptr<hostent> h, int err);

  vec<testslave*> _slaves;

  typedef struct client {
    unsigned id;
    testslave *slave;
    str fname;
    ptr<dhashclient_test> dhc;
    ihash_entry<client> hash_link;

    client(unsigned i, testslave *s, str f, ptr<dhashclient_test> d)
    {
      id = i;
      slave = s;
      fname = f;
      dhc = d;
    }
    ~client() {}
  } client;

  typedef ihash<unsigned, client, &client::id, &client::hash_link> clients_t;
  clients_t _clients;
};
