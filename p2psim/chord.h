#ifndef __CHORD_H
#define __CHORD_H

#include "protocol.h"
#include "consistenthash.h"

#include <vector>

#define CHORD_SUCC_NUM 3  //successor list contains CHORD_SUCC_NUM elements
#define STABLE_TIMER 500

#include "p2psim.h"

class LocTable;

class Chord : public Protocol {
public:
  typedef ConsistentHash::CHID CHID;
  struct IDMap {
    ConsistentHash::CHID id; //consistent hashing ID for the node
    IPAddress ip; //the IP address for the node
  };

  Chord(Node *n);
  virtual ~Chord();

  // Functions callable from events file.
  virtual void join(Args*);
  virtual void leave(Args*) {};
  virtual void crash(Args*) {};
  virtual void lookup(Args*);
  virtual void insert(Args*) {};

  struct get_predecessor_args {
    int dummy;
  };
  struct get_predecessor_ret {
    IDMap n;
  };

  struct get_successor_list_args {
    unsigned int m; //number of successors wanted
  };
  
  struct get_successor_list_ret {
    vector<IDMap> v;
  };

  struct notify_args {
    IDMap me;
  };
  struct notify_ret {
    int dummy;
  };
  struct next_args {
    CHID key;
    int m;
    IDMap who;
  };
  struct next_ret {
    bool done;
    vector<IDMap> v;
    IDMap next;
  };

  // RPC handlers.
  void get_predecessor_handler(get_predecessor_args *, get_predecessor_ret *);
  void get_successor_list_handler(get_successor_list_args *, get_successor_list_ret *);
  //void get_successor_list_handler(get_predecessor_args *, get_predecessor_ret *);
  void notify_handler(notify_args *, notify_ret *);
  void next_handler(next_args *, next_ret *);

  bool stabilized();
  virtual void dump();
  char *ts();

  virtual void stabilize();
  virtual void reschedule_stabilizer(void *);

protected:
  LocTable *loctable;
  IDMap me;

  virtual vector<IDMap> find_successors(CHID key, int m);
  void fix_predecessor();
  void fix_successor();
  void fix_successor_list();
};


class LocTable {

  public:

    LocTable(Chord::IDMap me) {
      _max = 3; 
      _succ_num = 1;

      // XXX: shouldn't the just be a new?
      ring.clear();
      ring.push_back(me);//ring[0] is always me
      ring.push_back(me);
      ring.push_back(me);
      ring[1].ip = 0; // ring[1] is always successor, currently null
      ring[2].ip = 0; //ring.back() is always predecessor, currently null
      pinlist.clear();

      _prev_chkp = 0;
      _stabilized = false;
      _changed = false;
    }; 

    ~LocTable() {
    }

    void resize(unsigned int max, unsigned int s) {
      _max = max; //this is not a general resize, it has to be called immediately after construction, it does not re-init
      _succ_num = s;
    };

    Chord::IDMap succ(unsigned int which);
    vector<Chord::IDMap> succs(unsigned int m);
    Chord::IDMap pred(Chord::CHID n);
    Chord::IDMap pred();
    Chord::IDMap next(Chord::CHID n);
    vector<Chord::IDMap> succ_for_key(Chord::CHID key);
    void checkpoint();
    bool stabilized() {return _stabilized;};

    void add_node(Chord::IDMap n);
    void del_node(Chord::IDMap n);
    void notify(Chord::IDMap n);
    void pin(Chord::CHID x);
    void dump();
/*
    struct idmapwrap {
      sklist_entry<idmapwrap> sortlink_;
      Chord::CHID id_;
      IPAddress ip_;
    };
    struct chidwrap {
      sklist_entry<chidwrap> sortlink_;
      Chord::CHID id_;
    };
    skiplist<idmapwrap, Chord::CHID, &idmapwrap::id_, &idmapwrap::sortlink_> loclist;  emil's skiplist needs sfs's keyfunc.h
    skiplist<chidwrap, Chord::CHID, &chidwrap::id_, &chidwrap::sortlink_> pinlist;
    */
  private:
    vector<Chord::IDMap> ring;
    vector<Chord::CHID> pinlist;
    unsigned int _succ_num;
    unsigned int _max;

    Time _prev_chkp;
    bool _stabilized;
    bool _changed;

    unsigned int evict(); //evict one node to make sure ring contains <= _max elements
};

#endif // __CHORD_H
