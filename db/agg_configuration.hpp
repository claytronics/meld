
#ifndef DB_AGG_CONFIGURATION_HPP
#define DB_AGG_CONFIGURATION_HPP

#include <ostream>

#include "mem/base.hpp"
#include "db/tuple.hpp"
#include "vm/defs.hpp"
#include "vm/types.hpp"

namespace db
{
   
class agg_configuration: public mem::base<agg_configuration>
{
private:

   simple_tuple_list values;
   bool changed;
   vm::tuple *corresponds;

   vm::tuple *generate_max_int(const vm::field_num) const;
   vm::tuple *generate_min_int(const vm::field_num) const;
   vm::tuple *generate_sum_int(const vm::field_num) const;
   vm::tuple *generate_sum_float(const vm::field_num) const;
   vm::tuple *generate_first(void) const;
   vm::tuple *generate_max_float(const vm::field_num) const;
   vm::tuple *generate_min_float(const vm::field_num) const;
   vm::tuple *do_generate(const vm::aggregate_type, const vm::field_num);

public:

   void print(std::ostream&) const;

   void generate(const vm::aggregate_type, const vm::field_num, simple_tuple_list&);

   const bool test(vm::tuple *, const vm::field_num) const;

   inline const bool has_changed(void) const { return changed; }
   inline const bool is_empty(void) const { return values.empty(); }

   void add_to_set(vm::tuple *, const vm::ref_count);

   explicit agg_configuration(void):
      changed(false), corresponds(NULL)
   {
   }

   ~agg_configuration(void);
};

std::ostream& operator<<(std::ostream&, const agg_configuration&);
}

#endif