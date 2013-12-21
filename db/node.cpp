
#include <iostream>
#include <assert.h>

#include "db/node.hpp"
#include "vm/state.hpp"
#include "utils/utils.hpp"
#include "debug/debug_handler.hpp"

using namespace db;
using namespace std;
using namespace vm;
using namespace utils;

namespace db
{

tuple_trie*
node::get_storage(const predicate* pred)
{
   simple_tuple_map::iterator it(tuples.find(pred->get_id()));

   if(it == tuples.end()) {
      //cout << "New trie for " << *pred << endl;
      tuple_trie *tr(new tuple_trie(pred));
      tuples[pred->get_id()] = tr;
      return tr;
   } else
      return it->second;
}

bool
node::add_tuple(vm::tuple *tpl, const derivation_count many, const depth_t depth)
{
   const predicate* pred(tpl->get_predicate());
   tuple_trie *tr(get_storage(pred));
   
   bool ret = tr->insert_tuple(tpl, many, depth);
   return ret;
}

node::delete_info
node::delete_tuple(vm::tuple *tuple, const derivation_count many, const depth_t depth)
{
   const predicate *pred(tuple->get_predicate());
   tuple_trie *tr(get_storage(pred));

   return tr->delete_tuple(tuple, many, depth);

}

agg_configuration*
node::add_agg_tuple(vm::tuple *tuple, const derivation_count many, const depth_t depth)
{
   const predicate *pred(tuple->get_predicate());
   predicate_id pred_id(pred->get_id());
   aggregate_map::iterator it(aggs.find(pred_id));
   tuple_aggregate *agg;

   if(it == aggs.end()) {
      agg = new tuple_aggregate(pred);
      aggs[pred_id] = agg;
   } else
      agg = it->second;

   return agg->add_to_set(tuple, many, depth);
}

agg_configuration*
node::remove_agg_tuple(vm::tuple *tuple, const derivation_count many, const depth_t depth)
{
   return add_agg_tuple(tuple, -many, depth);
}

simple_tuple_list
node::end_iteration(void)
{
   // generate possible aggregates
   simple_tuple_list ret;

   for(aggregate_map::iterator it(aggs.begin());
      it != aggs.end();
      ++it)
   {
      tuple_aggregate *agg(it->second);

      simple_tuple_list ls(agg->generate());

      ret.insert(ret.end(), ls.begin(), ls.end());
   }

   return ret;
}

tuple_trie::tuple_search_iterator
node::match_predicate(const predicate_id id) const
{
   simple_tuple_map::const_iterator it(tuples.find(id));

   if(it == tuples.end())
      return tuple_trie::tuple_search_iterator();
   
   const tuple_trie *tr(it->second);
   
   return tr->match_predicate();
}

tuple_trie::tuple_search_iterator
node::match_predicate(const predicate_id id, const match* m) const
{
   simple_tuple_map::const_iterator it(tuples.find(id));

   if(it == tuples.end())
      return tuple_trie::tuple_search_iterator();
   
   const tuple_trie *tr(it->second);
   
   return tr->match_predicate(m);
}

void
node::delete_all(const predicate*)
{
   assert(false);
}

void
node::delete_by_leaf(const predicate *pred, tuple_trie_leaf *leaf, const depth_t depth)
{
   tuple_trie *tr(get_storage(pred));

   tr->delete_by_leaf(leaf, depth);
}

void
node::assert_tries(void)
{
   for(simple_tuple_map::iterator it(tuples.begin()), end(tuples.end()); it != end; ++it) {
      tuple_trie *tr(it->second);
      tr->assert_used();
   }
}

void
node::delete_by_index(const predicate *pred, const match& m)
{
   tuple_trie *tr(get_storage(pred));

   tr->delete_by_index(m);

   aggregate_map::iterator it(aggs.find(pred->get_id()));

   if(it != aggs.end()) {
      tuple_aggregate *agg(it->second);
      agg->delete_by_index(m);
   }
}

size_t
node::count_total(const predicate_id id) const
{
   simple_tuple_map::const_iterator it(tuples.find(id));

   if(it == tuples.end())
      return 0;

   const tuple_trie *tr(it->second);

   return tr->size();
}

void
node::assert_end(void) const
{
   for(aggregate_map::const_iterator it(aggs.begin());
      it != aggs.end();
      ++it)
   {
      assert(it->second->no_changes());
   }
}

node::node(const node_id _id
#ifdef USERFRIENDLY
           , const node_id _trans
#endif
           ):
  id(_id),
#ifdef USERFRIENDLY
  translation(_trans), 
#endif
  owner(NULL),
  db(vm::All->PROGRAM), store(vm::All->PROGRAM), unprocessed_facts(false)
{
}

node::~node(void)
{
   for(simple_tuple_map::iterator it(tuples.begin()), end(tuples.end()); it != end; it++)
      delete it->second;
   for(aggregate_map::iterator it(aggs.begin()), end(aggs.end()); it != end; it++)
      delete it->second;
}

void
node::dump(ostream& cout) const
{
   cout << get_id() << endl;
   
   for(size_t i(0); i < All->PROGRAM->num_predicates(); ++i) {
      predicate *pred(All->PROGRAM->get_sorted_predicate(i));
      const tuple_list *ls(db.get_list(pred->get_id()));
      simple_tuple_map::const_iterator it(tuples.find(pred->get_id()));
      tuple_trie *tr = NULL;
      if(it != tuples.end())
         tr = it->second;

      vector<string> vec;
      
      if(tr && !tr->empty())
         vec = tr->get_print_strings();

      if(!ls->empty()) {
         for(tuple_list::const_iterator it(ls->begin()), end(ls->end()); it != end; ++it) {
            vec.push_back(to_string(*(*it)));
         }
      }
      sort(vec.begin(), vec.end());

      for(size_t i(0); i < vec.size(); ++i)
         cout << vec[i] << endl;
   }
}

void
node::print(ostream& cout) const
{
	if( !debugger::isInDebuggingMode()&&!debugger::isInSimDebuggingMode()&&
        !debugger::isInMpiDebuggingMode()){
	  cout << "--> node " << get_translated_id() << "/(id " << get_id()
        << ") (" << this << ") <--" << endl;
	} else {
	  cout << "CONTENTS AT NODE " << get_translated_id() << ":" << endl;
	}
   
   for(size_t i(0); i < vm::All->PROGRAM->num_predicates(); ++i) {
      predicate *pred(vm::All->PROGRAM->get_sorted_predicate(i));
      const tuple_list *ls(db.get_list(pred->get_id()));
      simple_tuple_map::const_iterator it(tuples.find(pred->get_id()));
      tuple_trie *tr = NULL;
      bool empty = true;
      if(it != tuples.end()) {
         tr = it->second;
         empty = tr->empty();
      }
      if(empty)
         empty = ls->empty();

      if(empty)
         continue;

      cout << " ";
      pred->print_simple(cout);
      cout << endl;
      vector<string> vec;
      
      if(tr && !tr->empty())
         vec = tr->get_print_strings();

      if(!ls->empty()) {
         for(tuple_list::const_iterator it(ls->begin()), end(ls->end()); it != end; ++it) {
            vec.push_back(to_string(*(*it)));
         }
      }
      sort(vec.begin(), vec.end());
      write_strings(vec, cout, 1);
   }
}

bool
node::empty(void) const {
    for(simple_tuple_map::const_iterator it(tuples.begin());
        it != tuples.end();
        ++it) {
        if(!(it->second)->empty())
            return false;
    }
    for(size_t i(0); i < db.num_lists; ++i) {
       const tuple_list *l(db.get_list(i));
       if(!l->empty())
          return false;
    }

    return true;
}

void
node::init(void)
{
}

ostream&
operator<<(ostream& cout, const node& node)
{
   node.print(cout);
   return cout;
}

}


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
