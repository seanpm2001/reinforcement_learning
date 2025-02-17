#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "constants.h"
#include "err_constants.h"
#include "object_factory.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(factory_tempate_usage)
{
  struct an_interface
  {
    virtual void do_something() = 0;
    virtual ~an_interface() {}
  };
  struct impl_A : public an_interface
  {
    void do_something() override {}
  };
  struct impl_B : public an_interface
  {
    explicit impl_B(int b) {}
    void do_something() override {}
  };

  auto b = 5;  // arbitrary variable to illustrate a point
  u::object_factory<an_interface, const u::configuration&> factory;

  auto create_A_fn = [](std::unique_ptr<an_interface>& pret, const u::configuration&, r::i_trace* trace,
                         r::api_status*) -> int
  {
    pret.reset(new impl_A());
    return r::error_code::success;
  };

  auto create_B_fn = [b](std::unique_ptr<an_interface>& pret, const u::configuration&, r::i_trace* trace,
                         r::api_status*) -> int
  {
    pret.reset(new impl_B(b));
    return r::error_code::success;
  };

  factory.register_type("A", create_A_fn);
  factory.register_type("B", create_B_fn);

  u::configuration cc;
  cc.set(r::name::MODEL_SRC, r::value::AZURE_STORAGE_BLOB);

  std::unique_ptr<an_interface> p_impl;
  auto scode = factory.create(p_impl, std::string("A"), cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);
  p_impl->do_something();
}
