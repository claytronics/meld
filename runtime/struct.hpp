
// do NOT include this file directly, please include runtime/objs.hpp

#ifndef RUNTIME_OBJS_HPP
#error "Please include runtime/objs.hpp instead"
#endif

class struct1: public mem::base
{
   private:

      utils::atomic<vm::ref_count> refs;
      vm::tuple_field *fields;
      vm::struct_type *typ;

   public:

      inline vm::struct_type *get_type(void) const { return typ; }

      inline bool zero_refs(void) const { return refs == 0; }

      inline void inc_refs(void)
      {
         refs++;
      }

      inline size_t get_size(void) const { return typ->get_size(); }
      
      inline void dec_refs(void)
      {
         assert(refs > 0);
         refs--;
         if(zero_refs())
            destroy();
      }

      inline void destroy(void)
      {
         assert(zero_refs());
         for(size_t i(0); i < get_size(); ++i) {
            decrement_runtime_data(fields[i], typ->get_type(i));
         }
         delete this;
      }

      inline void set_data(const size_t i, const vm::tuple_field& data)
      {
         fields[i] = data;
         increment_runtime_data(fields[i], typ->get_type(i));
      }

      inline vm::tuple_field get_data(const size_t i) const
      {
         return fields[i];
      }

      inline vm::tuple_field* get_ptr(const size_t i)
      {
         return fields + i;
      }

      explicit struct1(vm::struct_type *_typ): refs(0), typ(_typ) {
         assert(typ);
         fields = mem::allocator<vm::tuple_field>().allocate(get_size());
      }

      ~struct1(void) {
         mem::allocator<vm::tuple_field>().deallocate(fields, get_size());
      }
};
