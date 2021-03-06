// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_POLICY_HACK_POLICY_WATCHER_H_
#define REMOTING_HOST_POLICY_HACK_POLICY_WATCHER_H_

#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "components/policy/core/common/policy_service.h"

namespace base {
class SingleThreadTaskRunner;
class TimeDelta;
class WaitableEvent;
}  // namespace base

namespace remoting {
namespace policy_hack {

// Watches for changes to the managed remote access host policies.
// If StartWatching() has been called, then before this object can be deleted,
// StopWatching() have completed (the provided |done| event must be signaled).
class PolicyWatcher {
 public:
  // Called first with all policies, and subsequently with any changed policies.
  typedef base::Callback<void(scoped_ptr<base::DictionaryValue>)>
      PolicyUpdatedCallback;

  // Called after detecting malformed policies.
  typedef base::Callback<void()> PolicyErrorCallback;

  explicit PolicyWatcher(
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  virtual ~PolicyWatcher();

  // This guarantees that the |policy_updated_callback| is called at least once
  // with the current policies.  After that, |policy_updated_callback| will be
  // called whenever a change to any policy is detected. It will then be called
  // only with the changed policies.
  //
  // |policy_error_callback| will be called when malformed policies are detected
  // (i.e. wrong type of policy value, or unparseable files under
  // /etc/opt/chrome/policies/managed).
  // When called, the |policy_error_callback| is responsible for mitigating the
  // security risk of running with incorrectly formulated policies (by either
  // shutting down or locking down the host).
  // After calling |policy_error_callback| PolicyWatcher will continue watching
  // for policy changes and will call |policy_updated_callback| when the error
  // is recovered from and may call |policy_error_callback| when new errors are
  // found.
  virtual void StartWatching(
      const PolicyUpdatedCallback& policy_updated_callback,
      const PolicyErrorCallback& policy_error_callback);

  // Should be called after StartWatching() before the object is deleted. Calls
  // should wait for |stopped_callback| to be called before deleting it.
  virtual void StopWatching(const base::Closure& stopped_callback);

  // Implemented by each platform.  |task_runner| should be an IO message loop.
  // |policy_service| is currently only used on ChromeOS.  The caller must
  // ensure that |policy_service| remains valid for the lifetime of
  // PolicyWatcher.
  static scoped_ptr<PolicyWatcher> Create(
      policy::PolicyService* policy_service,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);

  // The name of the NAT traversal policy.
  static const char kNatPolicyName[];

  // The name of the policy for requiring 2-factor authentication.
  static const char kHostRequireTwoFactorPolicyName[];

  // The name of the host domain policy.
  static const char kHostDomainPolicyName[];

  // The name of the username policy. This policy is ignored on Windows.
  // This policy is currently considered 'internal only' and so is not
  // documented in policy_templates.json.
  static const char kHostMatchUsernamePolicyName[];

  // The name of the policy that controls the host talkgadget prefix.
  static const char kHostTalkGadgetPrefixPolicyName[];

  // The name of the policy for requiring curtain-mode.
  static const char kHostRequireCurtainPolicyName[];

  // The names of the policies for token authentication URLs.
  static const char kHostTokenUrlPolicyName[];
  static const char kHostTokenValidationUrlPolicyName[];
  static const char kHostTokenValidationCertIssuerPolicyName[];

  // The name of the policy for disabling PIN-less authentication.
  static const char kHostAllowClientPairing[];

  // The name of the policy for disabling gnubbyd forwarding.
  static const char kHostAllowGnubbyAuthPolicyName[];

  // The name of the policy for allowing use of relay servers.
  static const char kRelayPolicyName[];

  // The name of the policy that restricts the range of host UDP ports.
  static const char kUdpPortRangePolicyName[];

  // The name of the policy for overriding policies, for use in testing.
  static const char kHostDebugOverridePoliciesName[];

 protected:
  virtual void StartWatchingInternal() = 0;
  virtual void StopWatchingInternal() = 0;
  virtual void Reload() = 0;

  // Used to check if the class is on the right thread.
  bool OnPolicyWatcherThread() const;

  // Takes the policy dictionary from the OS specific store and extracts the
  // relevant policies.
  void UpdatePolicies(const base::DictionaryValue* new_policy);

  // Signals policy error to the registered |PolicyErrorCallback|.
  void SignalPolicyError();

  // Called whenever a transient error occurs during reading of policy files.
  // This will increment a counter, and will trigger a call to
  // SignalPolicyError() only after a threshold count is reached.
  // The counter is reset whenever policy has been successfully read.
  void SignalTransientPolicyError();

  // Used for time-based reloads in case something goes wrong with the
  // notification system.
  void ScheduleFallbackReloadTask();
  void ScheduleReloadTask(const base::TimeDelta& delay);

  // Returns a DictionaryValue containing the default values for each policy.
  const base::DictionaryValue& Defaults() const;

 private:
  void StopWatchingOnPolicyWatcherThread();
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  PolicyUpdatedCallback policy_updated_callback_;
  PolicyErrorCallback policy_error_callback_;
  int transient_policy_error_retry_counter_;

  scoped_ptr<base::DictionaryValue> old_policies_;
  scoped_ptr<base::DictionaryValue> default_values_;
  scoped_ptr<base::DictionaryValue> bad_type_values_;

  // Allows us to cancel any inflight FileWatcher events or scheduled reloads.
  base::WeakPtrFactory<PolicyWatcher> weak_factory_;
};

}  // namespace policy_hack
}  // namespace remoting

#endif  // REMOTING_HOST_POLICY_HACK_POLICY_WATCHER_H_
