
#ifndef PROCESS_MACHINE_HPP
#define PROCESS_MACHINE_HPP

#include "conf.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#if defined(TARGET_mpi)
# include <boost/mpi.hpp>
#endif
#include "db/database.hpp"
#include "db/tuple.hpp"
#include "db/node.hpp"
#include "sched/types.hpp"
#include "stat/slice_set.hpp"
#include "vm/state.hpp"
#include "sched/base.hpp"
#include "api/api.hpp"

extern vm::all* vm::All;

namespace process
{

class machine
{
private:
   
   vm::all *all;
   const std::string filename;
   const sched::scheduler_type sched_type;
   
   boost::thread *alarm_thread;
   statistics::slice_set slices;
   
	void execute_const_code(void);
   void deactivate_signals(void);
   void slice_function(void);
   void set_timer(void);
   
public:
   
   sched::scheduler_type get_sched_type(void) const { return sched_type; }
   
   sched::base *get_scheduler(const vm::process_id id) { return this->all->ALL_THREADS[id]; }

   vm::all *get_all(void) const { return vm::All; }
   
   bool same_place(const db::node::node_id, const db::node::node_id) const;
   
   void run_action(sched::base *, db::node *, vm::tuple *);
   
   inline void route(db::node *from, sched::base *sched_caller, const db::node::node_id id, vm::tuple* tpl,
         const vm::ref_count count, const vm::depth_t depth, const vm::uint_val delay = 0)
   {
      assert(sched_caller != NULL);
      if (api::onLocalVM(id)){
         /* Belongs to the same process, does not require MPI */
         const vm::predicate *pred(tpl->get_predicate());
#ifdef USE_REAL_NODES
         db::node *node((db::node*)id);
#else
         assert(id <= vm::All->DATABASE->max_id());
         db::node *node(vm::All->DATABASE->find_node(id));
#endif

         if(delay > 0)
            sched_caller->new_work_delay(from, node, tpl, count, depth, delay);
         else if(pred->is_action_pred())
            run_action(sched_caller, node, tpl);
         else
            sched_caller->new_work(from, node, tpl, count, depth);
      } else {
         /* Send to the correct process */
         db::simple_tuple *stpl(new db::simple_tuple(tpl, count, depth));
         api::sendMessage(from,id,stpl);
      }
   }
   
	void init_thread(sched::base *);
   void start(void);
   
   explicit machine(const std::string&, const size_t, const sched::scheduler_type, const vm::machine_arguments& args = vm::machine_arguments(), const std::string& data_file = std::string());
               
   ~machine(void);
};

class machine_error : public std::runtime_error {
 public:
    explicit machine_error(const std::string& msg) :
      std::runtime_error(msg)
    {}
  };

}

#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
