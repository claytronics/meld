
#ifndef SCHED_THREADS_HPP
#define SCHED_THREADS_HPP

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

#include "sched/static.hpp"
#include "vm/defs.hpp"

namespace sched
{
   
class threads_static : public sched::sstatic
{
private:
   
   utils::byte _pad_threads1[128];
   
   enum {
      PROCESS_ACTIVE,
      PROCESS_INACTIVE
   } process_state;
   
   utils::byte _pad_threads2[128];
   
   boost::mutex mutex;
   
   utils::byte _pad_threads3[128];
   
   typedef unsafe_queue_count<work_unit> queue_free_work;
   std::vector<queue_free_work, mem::allocator<queue_free_work> > buffered_work;
   
   void make_active(void);
   void make_inactive(void);
   bool all_buffers_emptied(void) const;
   void flush_this_queue(queue_free_work&, threads_static *);
   void flush_buffered(void);
   
protected:
   
   virtual void assert_end_iteration(void) const;
   virtual void assert_end(void) const;
   
   virtual bool busy_wait(void);
   
   virtual void begin_get_work(void);
   
   virtual void work_found(void);
   
public:
   
   virtual void new_work(db::node *, db::node *, const db::simple_tuple*, const bool is_agg = false);
   virtual void new_work_other(sched::base *, db::node *, const db::simple_tuple *);
   virtual void new_work_remote(process::remote *, const vm::process_id, process::message *);
   
   virtual void init(const size_t);
   virtual void end(void);
   virtual bool terminate_iteration(void);
   virtual bool get_work(work_unit&);
   
   threads_static *find_scheduler(const db::node::node_id);
   
   static std::vector<threads_static*>& start(const size_t num_threads);
   
   explicit threads_static(const vm::process_id);
   
   virtual ~threads_static(void);
};

}

#endif
