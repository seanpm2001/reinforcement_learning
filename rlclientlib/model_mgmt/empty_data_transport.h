#pragma once
#include "model_mgmt.h"

namespace reinforcement_learning
{
namespace model_management
{
class empty_data_transport : public i_data_transport
{
public:
  int get_data(model_data& ret, api_status* status) override;

private:
};
}  // namespace model_management
}  // namespace reinforcement_learning
