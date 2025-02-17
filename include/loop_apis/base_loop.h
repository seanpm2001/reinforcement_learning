/**
 * @brief RL Inference API definition.
 *
 * @file base_loop.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "sender.h"

#include <cstring>
#include <functional>
#include <memory>

#define INIT_CHECK()                                                                                     \
  do {                                                                                                   \
    if (!_initialized)                                                                                   \
    {                                                                                                    \
      RETURN_ERROR_ARG(nullptr, status, not_initialized, "Library not initialized. Call init() first."); \
    }                                                                                                    \
  } while (0);

namespace reinforcement_learning
{
//// Forward declarations ////////
class live_model_impl;   //
class ranking_response;  //
class api_status;        //
                         //
namespace model_management
{                        //
class i_data_transport;  //
class i_model;           //
}  // namespace model_management
   //
namespace utility
{                     //
class configuration;  //
}  // namespace utility
//////////////////////////////////

struct str_view
{
  str_view(const char* str, size_t size) : str(str), size(size) {}
  str_view(const char* str) : str(str), size(str == nullptr ? 0 : strlen(str)) {}
  const char* str;
  size_t size;
};

// Reinforcement learning client
/**
 * @brief Interface class for the Inference API.
 *
 * - (1) Instantiate and Initialize
 * - (2) choose_rank() to choose an action from a list of actions
 * - (3) report_outcome() to provide feedback on chosen action
 */
class base_loop
{
public:
  /**
   * @brief Error callback function.
   * When base_loop is constructed, a background error callback and a
   * context (void*) is registered. If there is an error in the background thread,
   * error callback will get invoked with api_status and the context (void*).
   *
   * NOTE: Error callback will get invoked in a background thread.
   */
  using error_fn = void (*)(const api_status&, void*);

  /**
   * @brief Construct a new live model object.
   *
   * @param config Name-Value based configuration
   * @param fn Error callback for handling errors in background thread
   * @param err_context Context passed back during Error callback
   * @param t_factory Transport factory.  The default transport factory is initialized with a
   *                  REST based transport that gets data from an Azure storage account
   * @param m_factory Model factory.  The default model factory hydrates vw models
   *                    used for local inference.
   * @param sender_factory Sender factory.  The default factory provides two senders, one for
   *                       interaction and the other for observation which logs to Event Hub.
   */
  explicit base_loop(const utility::configuration& config, error_fn fn = nullptr, void* err_context = nullptr,
      trace_logger_factory_t* trace_factory = &trace_logger_factory,
      data_transport_factory_t* t_factory = &data_transport_factory, model_factory_t* m_factory = &model_factory,
      sender_factory_t* s_factory = &sender_factory,
      time_provider_factory_t* time_prov_factory = &time_provider_factory);

  /**
   * @brief Construct a new live model object.
   *
   * @param config Name-Value based configuration
   * @param error_cb Error callback that takes no context
   * @param t_factory Transport factory.  The default transport factory is initialized with a
   *                  REST based transport that gets data from an Azure storage account
   * @param m_factory Model factory.  The default model factory hydrates vw models
   *                    used for local inference.
   * @param sender_factory Sender factory.  The default factory provides two senders, one for
   *                       interaction and the other for observation which logs to Event Hub.
   */
  explicit base_loop(const utility::configuration& config, std::function<void(const api_status&)> error_cb,
      trace_logger_factory_t* trace_factory = &trace_logger_factory,
      data_transport_factory_t* t_factory = &data_transport_factory, model_factory_t* m_factory = &model_factory,
      sender_factory_t* s_factory = &sender_factory,
      time_provider_factory_t* time_prov_factory = &time_provider_factory);

  /**
   * @brief Initialize inference library.
   * Initialize the library and start the background threads used for
   * model managment and sending actions and outcomes to the online trainer
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int init(api_status* status = nullptr);

  /**
   * @brief Report that action was taken.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_action_taken(str_view event_id, api_status* status = nullptr);

  /**
   * @brief Report that action was taken.
   *
   * @param primary_id  The unique primary_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param secondary_id Index of the partial outcome.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_action_taken(str_view primary_id, str_view secondary_id, api_status* status = nullptr);

  /*
   * @brief Refreshes the model if it has background refresh disabled.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int refresh_model(api_status* status = nullptr);

  /**
   * @brief Error callback function.
   * When base_loop is constructed, a background error callback and a
   * context (void*) is registered. If there is an error in the background thread,
   * error callback will get invoked with api_status and the context (void*).
   * This error callback is typed by the context used in the callback.
   *
   * NOTE: Error callback will get invoked in a background thread.
   * @tparam ErrCntxt Context type used when the error callback is invoked
   */
  template <typename ErrCntxt>
  using error_fn_t = void (*)(const api_status&, ErrCntxt*);

  /**
   * @brief Construct a new live model object.
   *
   * @tparam ErrCntxt Context type used in error callback.
   * @param config Name-Value based configuration
   * @param fn Error callback for handling errors in background thread
   * @param err_context Context passed back during Error callback
   * @param t_factory Transport factory.  The default transport factory is initialized with a
   *                  REST based transport that gets data from an Azure storage account
   * @param m_factory Model factory.  The default model factory hydrates vw models
   *                    used for local inference.
   * @param sender_factory Sender factory.  The default factory provides two senders, one for
   *                       interaction and the other for observation which logs to Event Hub.
   */
  template <typename ErrCntxt>
  explicit base_loop(const utility::configuration& config, error_fn_t<ErrCntxt> fn, ErrCntxt* err_context = nullptr,
      trace_logger_factory_t* trace_factory = &trace_logger_factory,
      data_transport_factory_t* t_factory = &data_transport_factory, model_factory_t* m_factory = &model_factory,
      sender_factory_t* s_factory = &sender_factory,
      time_provider_factory_t* time_prov_factory = &time_provider_factory);

  /**
   * @brief Move constructor for live model object.
   */
  base_loop(base_loop&& other) noexcept;

  /**
   * @brief Move assignment operator swaps implementation.
   */
  base_loop& operator=(base_loop&& other) noexcept;

  base_loop(
      const base_loop&) = delete;  //! Prevent accidental copy, since destructor will deallocate the implementation
  base_loop& operator=(
      base_loop&) = delete;  //! Prevent accidental copy, since destructor will deallocate the implementation

  ~base_loop();

protected:
  std::unique_ptr<live_model_impl>
      _pimpl;                 //! The actual implementation details are forwarded to this object (PIMPL pattern)
  bool _initialized = false;  //! Guard to ensure that base_loop is properly initialized. i.e. init() was called
                              //! and successfully initialized.
};

template <typename ErrCntxt>
base_loop::base_loop(const utility::configuration& config, error_fn_t<ErrCntxt> fn, ErrCntxt* err_context,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
    : base_loop(config, std::bind(fn, std::placeholders::_1, err_context), trace_factory, t_factory, m_factory,
          s_factory, time_prov_factory)
{
}
}  // namespace reinforcement_learning
