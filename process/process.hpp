
#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <list>
#include <stdexcept>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "vm/program.hpp"
#include "vm/state.hpp"
#include "db/node.hpp"
#include "db/database.hpp"
#include "utils/interval.hpp"

namespace process
{
   
typedef struct {
   db::node *work_node;
   const db::simple_tuple *work_tpl;
   bool agg;
} work_unit;

class process
{
public:
   
   typedef unsigned short process_id;
   
private:
   
   static boost::mutex remote_mutex;
   
   typedef std::list<db::node*> list_nodes;
   typedef std::list<work_unit> work_queue;
   
   utils::interval<db::node::node_id> *nodes_interval;
   boost::thread *thread;
   process_id id;
   
   boost::mutex mutex;
   list_nodes nodes;
   vm::state state;
   work_queue queue;
   volatile bool more_work;
   
   enum {
      PROCESS_ACTIVE,
      PROCESS_INACTIVE
   } process_state;
   
   bool ended;
   bool agg_checked;
   
   void generate_aggs(void);
   void do_tuple_add(db::node *, vm::tuple *, const vm::ref_count);
   void do_work(db::node *, const db::simple_tuple *, const bool ignore_agg = false);
   bool busy_wait(void);
   bool get_work(work_unit&);
   void fetch_work(void);
   void do_loop(void);
   void loop(void);
   void update_remotes(void);

public:
   
   static db::database *DATABASE;
   static vm::program *PROGRAM;
   
   void add_node(db::node*);
   
   void enqueue_work(db::node*, const db::simple_tuple*, const bool is_agg = false);
   
   const process_id get_id(void) const { return id; }
   
   void start(void);
   
   void notify(void);
   
   bool owns_node(const db::node::node_id& id) const { return nodes_interval->between(id); }
   
   void finished(void) { thread->join(); }
   
   bool has_ended(void) const { return ended; }
   
   void print(std::ostream&) const;
   
   explicit process(const process_id&);
   
   ~process(void);
};

std::ostream& operator<<(std::ostream&, const process&);

class process_exec_error : public std::runtime_error {
 public:
    explicit process_exec_error(const std::string& msg) :
         std::runtime_error(msg)
    {}
};

}

#endif
