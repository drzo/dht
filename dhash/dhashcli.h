
#include <sys/types.h>

// SFS includes
#include <ihash.h>
#include <refcnt.h>
#include <vec.h>

typedef callback<void, dhash_stat, chordID>::ref cbinsert_t;
typedef callback<void, dhash_stat, vec<chordID> >::ref cbinsert_path_t;
typedef callback<void, dhash_stat, vec<chord_node>, route>::ref dhashcli_lookupcb_t;

// Forward declarations
class vnode;
class dhash;
class route_factory;
class dhash_block;
class route_iterator;

class dhashcli {
  ptr<vnode> clntnode;
  bool do_cache;
  int server_selection_mode;
  dhash *dh;
  ptr<route_factory> r_factory;

  struct rcv_state {
    ihash_entry <rcv_state> link;
    blockID key;
    route r;
    vec<timespec> times;
    
    int incoming_rpcs;
    
    vec<chord_node> succs;
    size_t nextsucc;
    
    vec<str> frags;
    vec<cb_ret> callbacks;

    void timemark () {
      timespec x;
      clock_gettime (CLOCK_REALTIME, &x);
      times.push_back (x);
    }
    
    void complete (dhash_stat s, ptr<dhash_block> b) {
      for (u_int i = 0; i < callbacks.size (); i++)
	(callbacks[i]) (s, b, r);
      delete this;
    }
      
    rcv_state (blockID key) : key (key), incoming_rpcs (0), nextsucc (0) {
      timemark ();
    }
  };

  ihash<blockID, rcv_state, &rcv_state::key, &rcv_state::link, bhashID> rcvs;

  // State for a fragment store
  struct sto_state {
    ref<dhash_block> block;
    vec<chord_node> succs;
    cbinsert_path_t cb;
    
    u_int out;
    u_int good;
    
    sto_state (ref<dhash_block> b, cbinsert_path_t x) :
      block (b), cb (x), out (0), good (0) {}
  };


private:
  void doRPC (chordID ID, const rpc_program &prog, int procno, ptr<void> in, 
	      void *out, aclnt_cb cb);

  void lookup_findsucc_cb (chordID blockID, dhashcli_lookupcb_t cb,
			   vec<chord_node> s, route path, chordstat err);
    
  void insert_lookup_cb (ref<dhash_block> block, cbinsert_path_t cb, 
			 dhash_stat status, vec<chord_node> succs, route r);
  void insert_store_cb (ref<sto_state> ss, route r, u_int i, 
			u_int nstores, u_int min_needed,
			dhash_stat err, chordID id, bool present);
  void fetch_frag (rcv_state *rs);

  void retrieve_frag_hop_cb (blockID blockID, route_iterator *ci, bool done);
  void retrieve_lookup_cb (blockID blockID, dhash_stat status,
			   vec<chord_node> succs, route r);
  void retrieve_fetch_cb (blockID blockID, u_int i,
			  ptr<dhash_block> block);

  void insert_succlist_cb (ref<dhash_block> block, cbinsert_path_t cb,
			   chordID guess,
			   vec<chord_node> succs, chordstat status);
  void retrieve_block_hop_cb (blockID blockID, route_iterator *ci,
			     int options, int retries, ptr<chordID> guess,
			     bool done);
  void retrieve_dl_or_walk_cb (blockID blockID, dhash_stat status, int options,
			       int retries, ptr<chordID> guess,
			       ptr<dhash_block> blk);
 public:
  dhashcli (ptr<vnode> node, dhash *dh, ptr<route_factory> r_factory, 
	    bool do_cache, int ss_mode = 1);


  void retrieve (blockID blockID, cb_ret cb, 
		 int options = 0, 
		 ptr<chordID> guess = NULL);

  void insert (ref<dhash_block> block, cbinsert_path_t cb, 
	       int options = 0, 
	       ptr<chordID> guess = NULL);

  void lookup (chordID blockID, dhashcli_lookupcb_t cb);
};

