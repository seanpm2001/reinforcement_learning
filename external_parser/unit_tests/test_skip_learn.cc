#include <boost/test/unit_test.hpp>

#include "joiners/example_joiner.h"
#include "parse_example_binary.h"
#include "test_common.h"
#include "vw/config/options_cli.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"

void should_not_add_examples(std::string& infile, v2::ProblemType problem_type = v2::ProblemType_CB)
{
  std::string command;
  switch (problem_type)
  {
    case v2::ProblemType_CB:
      command = "--quiet --binary_parser --cb_explore_adf";
      break;
    case v2::ProblemType_CCB:
      command = "--quiet --binary_parser --ccb_explore_adf";
      break;
    case v2::ProblemType_CA:
      command = "--quiet --binary_parser --cats 4 --min_value 1 --max_value 100 --bandwidth 1";
      break;
    case v2::ProblemType_SLATES:
      command = "--quiet --binary_parser --ccb_explore_adf --slates";
      break;
    case reinforcement_learning::messages::flatbuff::v2::ProblemType_UNKNOWN:
    case reinforcement_learning::messages::flatbuff::v2::ProblemType_MULTISTEP:
      THROW("Unknown problem type");
      break;
  }

  std::string infile_name = get_test_files_location() + "skip_learn/" + infile;

  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(command + " -d " + infile_name));
  auto vw = VW::external::initialize_with_binary_parser(std::move(options));

  VW::multi_ex examples;
  examples.push_back(VW::new_unused_example(*vw));

  while (vw->parser_runtime.example_parser->reader(vw.get(), vw->parser_runtime.example_parser->input, examples) > 0)
  {
    continue;
  }

  BOOST_CHECK_EQUAL(examples.size(), 1);
  BOOST_CHECK_EQUAL(examples[0]->indices.size(), 0);  // newline example

  clear_examples(examples, vw.get());
  VW::finish(*vw, false);
}

void should_add_examples(std::string& infile, v2::ProblemType problem_type = v2::ProblemType_CB)
{
  std::string command;
  switch (problem_type)
  {
    case v2::ProblemType_CB:
      command = "--quiet --binary_parser --cb_explore_adf";
      break;
    case v2::ProblemType_CCB:
      command = "--quiet --binary_parser --ccb_explore_adf";
      break;
    case v2::ProblemType_CA:
      command = "--quiet --binary_parser --cats 4 --min_value 1 --max_value 100 --bandwidth 1";
      break;
    case v2::ProblemType_SLATES:
      command = "--quiet --binary_parser --ccb_explore_adf --slates";
      break;
    default:
      assert(false);
      break;
  }

  std::string infile_name = get_test_files_location() + "skip_learn/" + infile;
  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(command + " -d " + infile_name));
  auto vw = VW::external::initialize_with_binary_parser(std::move(options));

  VW::multi_ex examples;
  examples.push_back(VW::new_unused_example(*vw));

  while (vw->parser_runtime.example_parser->reader(vw.get(), vw->parser_runtime.example_parser->input, examples) > 0)
  {
    continue;
  }

  switch (problem_type)
  {
    case v2::ProblemType_CB:
      BOOST_CHECK_EQUAL(examples.size(), 4);
      BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
      BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0);  // newline example
      break;
    case v2::ProblemType_CCB:
      // learn/predict isn't called in the unit test but cleanup examples expects
      // shared pred to be set
      examples[0]->pred.decision_scores.resize(1);
      examples[0]->pred.decision_scores[0].push_back({0, 0.f});

      // shared, two actions, two slots and one empty (end of multiline)
      BOOST_CHECK_EQUAL(examples.size(), 6);
      // first example is shared example
      BOOST_CHECK_EQUAL(VW::reductions::ccb::ec_is_example_header(*examples[0]), true);
      // next two examples are actions
      BOOST_CHECK_EQUAL(examples[1]->l.conditional_contextual_bandit.type == VW::ccb_example_type::ACTION, true);
      BOOST_CHECK_EQUAL(examples[2]->l.conditional_contextual_bandit.type == VW::ccb_example_type::ACTION, true);
      // next two examples are slots
      BOOST_CHECK_EQUAL(examples[3]->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT, true);
      BOOST_CHECK_EQUAL(examples[4]->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT, true);
      // last example is empty
      BOOST_CHECK_EQUAL(VW::example_is_newline(*examples[5]), true);
      break;
    case v2::ProblemType_CA:
      BOOST_CHECK_EQUAL(examples.size(), 1);
      BOOST_CHECK_EQUAL(examples[0]->l.cb_cont.costs.size(), 1);
      BOOST_CHECK_EQUAL(examples[0]->indices[0], 'R');
      BOOST_CHECK_EQUAL(examples[0]->feature_space[(int)'R'].sum_feat_sq, 78 * 78);
      break;
    case v2::ProblemType_SLATES:
      BOOST_CHECK_EQUAL(examples.size(), 9);
      BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
      BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[3]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[3]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[4]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[4]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[5]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[5]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[6]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[6]->indices[0], 'S');
      BOOST_CHECK_EQUAL(examples[7]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[7]->indices[0], 'S');
      BOOST_CHECK_EQUAL(examples[8]->indices.size(), 0);  // newline example

      set_slates_label(examples);  // set the shared pred label for clear example
      break;
    default:
      assert(false);
      break;
  }

  clear_examples(examples, vw.get());
  VW::finish(*vw, false);
}

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_cb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action and 1 inactivated observation
  std::string test_file = "cb/deferred_action_without_activation.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb/deferred_action_with_activation.fb";
  should_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(only_add_learnable_events_to_examples)
{
  // file contains 2 interaction, first interaction is deferred action.
  // observations are not activated
  // only second joined event should be added to examples
  std::string test_file = "cb/mixed_deferred_action_events.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_compressed_and_deduped_cb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action and 1 inactivated observation
  std::string test_file = "cb/deferred_action_without_activation_deduped.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb/deferred_action_with_activation_deduped.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_ccb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action
  // 4 outcome events for each slot
  std::string test_file = "ccb/deferred_action_without_activation.fb";
  should_not_add_examples(test_file, v2::ProblemType_CCB);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "ccb/deferred_action_with_activation.fb";
  should_add_examples(test_file, v2::ProblemType_CCB);
}

BOOST_AUTO_TEST_CASE(only_add_learnabled_events_to_examples)
{
  // file contains 2 interactions, first is deferred action
  // 4 outcome events matching with first interaction
  // 4 outcome events matching with second interaction
  // only the second joined event should be added to examples
  std::string test_file = "ccb/mixed_deferred_action_events.fb";
  should_add_examples(test_file, v2::ProblemType_CCB);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_compressed_and_deduped_ccb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action
  // 4 outcome events for each slot
  std::string test_file = "ccb/deferred_action_without_activation_deduped.fb";
  should_not_add_examples(test_file, v2::ProblemType_CCB);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 action taken outcome event
  std::string test_file = "ccb/deferred_action_with_activation_deduped.fb";
  should_add_examples(test_file, v2::ProblemType_CCB);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_ca_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action and 1 inactivated observation
  std::string test_file = "ca/deferred_action_without_activation.fb";
  should_not_add_examples(test_file, v2::ProblemType_CA);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "ca/deferred_action_with_activation.fb";
  should_add_examples(test_file, v2::ProblemType_CA);
}

BOOST_AUTO_TEST_CASE(only_add_learnable_events_to_examples)
{
  // file contains 2 interaction, first interaction is deferred action.
  // observations are not activated
  // only second joined event should be added to examples
  std::string test_file = "ca/mixed_deferred_action_events.fb";
  should_add_examples(test_file, v2::ProblemType_CA);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_compressed_and_deduped_ca_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action and 1 inactivated observation
  std::string test_file = "ca/deferred_action_without_activation_deduped.fb";
  should_not_add_examples(test_file, v2::ProblemType_CA);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "ca/deferred_action_with_activation_deduped.fb";
  should_add_examples(test_file, v2::ProblemType_CA);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_slates_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_regular_outcome_is_not_learnable)
{
  // file contains 1 interaction with deferred action
  // 4 outcome events for each slot
  std::string test_file = "slates/deferred_action_without_activation.fb";
  should_not_add_examples(test_file, v2::ProblemType_SLATES);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable)
{
  // file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "slates/deferred_action_with_activation.fb";
  should_add_examples(test_file, v2::ProblemType_SLATES);
}

BOOST_AUTO_TEST_CASE(only_add_learnabled_events_to_examples)
{
  // file contains 2 interactions, first is deferred action
  // 4 outcome events matching with first interaction
  // 4 outcome events matching with second interaction
  // only the second joined event should be added to examples
  std::string test_file = "slates/mixed_deferred_action_events.fb";
  should_add_examples(test_file, v2::ProblemType_SLATES);
}
BOOST_AUTO_TEST_SUITE_END()
