// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/bluetooth_media_endpoint_service_provider.h"

#include <string>

#include "base/bind.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/platform_thread.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/fake_bluetooth_media_endpoint_service_provider.h"
#include "dbus/bus.h"
#include "dbus/exported_object.h"
#include "dbus/message.h"

namespace {

// TODO(mcchou): Move these constants to dbus/service_constants.h.
// Bluetooth Media Endpoint service identifier.
const char kBluetoothMediaEndpointInterface[] = "org.bluez.MediaEndpoint1";

// Bluetooth Media Endpoint methods.
const char kSetConfiguration[] = "SetConfiguration";
const char kSelectConfiguration[] = "SelectConfiguration";
const char kClearConfiguration[] = "ClearConfiguration";
const char kRelease[] = "Release";

}  // namespace

namespace chromeos {

// The BluetoothMediaEndopintServiceProvider implementation used in production.
class CHROMEOS_EXPORT BluetoothMediaEndpointServiceProviderImpl
    : public BluetoothMediaEndpointServiceProvider {
 public:
  BluetoothMediaEndpointServiceProviderImpl(dbus::Bus* bus,
                                            const dbus::ObjectPath& object_path,
                                            Delegate* delegate)
      : origin_thread_id_(base::PlatformThread::CurrentId()),
        bus_(bus),
        delegate_(delegate),
        object_path_(object_path),
        weak_ptr_factory_(this) {
    VLOG(1) << "Creating Bluetooth Media Endpoint: " << object_path_.value();
    DCHECK(bus_);
    DCHECK(delegate_);
    DCHECK(object_path_.IsValid());

    exported_object_ = bus_->GetExportedObject(object_path_);

    exported_object_->ExportMethod(
        kBluetoothMediaEndpointInterface,
        kSetConfiguration,
        base::Bind(
            &BluetoothMediaEndpointServiceProviderImpl::SetConfiguration,
            weak_ptr_factory_.GetWeakPtr()),
        base::Bind(&BluetoothMediaEndpointServiceProviderImpl::OnExported,
                   weak_ptr_factory_.GetWeakPtr()));

    exported_object_->ExportMethod(
        kBluetoothMediaEndpointInterface,
        kSelectConfiguration,
        base::Bind(
            &BluetoothMediaEndpointServiceProviderImpl::SelectConfiguration,
            weak_ptr_factory_.GetWeakPtr()),
        base::Bind(&BluetoothMediaEndpointServiceProviderImpl::OnExported,
                   weak_ptr_factory_.GetWeakPtr()));

    exported_object_->ExportMethod(
        kBluetoothMediaEndpointInterface,
        kClearConfiguration,
        base::Bind(
            &BluetoothMediaEndpointServiceProviderImpl::ClearConfiguration,
            weak_ptr_factory_.GetWeakPtr()),
        base::Bind(&BluetoothMediaEndpointServiceProviderImpl::OnExported,
                   weak_ptr_factory_.GetWeakPtr()));

    exported_object_->ExportMethod(
        kBluetoothMediaEndpointInterface,
        kRelease,
        base::Bind(&BluetoothMediaEndpointServiceProviderImpl::Release,
                   weak_ptr_factory_.GetWeakPtr()),
        base::Bind(&BluetoothMediaEndpointServiceProviderImpl::OnExported,
                   weak_ptr_factory_.GetWeakPtr()));
  }

  ~BluetoothMediaEndpointServiceProviderImpl() override {
    VLOG(1) << "Cleaning up Bluetooth Media Endpoint: "
            << object_path_.value();

    bus_->UnregisterExportedObject(object_path_);
  }

 private:
  // Returns true if the current thread is on the origin thread, false
  // otherwise.
  bool OnOriginThread() const {
    return base::PlatformThread::CurrentId() == origin_thread_id_;
  }

  // Called by dbus:: when a method is exported.
  void OnExported(const std::string& interface_name,
                  const std::string& method_name,
                  bool success) {
    LOG_IF(WARNING, !success) << "Failed to export "
                              << interface_name << "." << method_name;
  }

  // Called by dbus:: when the remote device connects to the Media Endpoint.
  void SetConfiguration(dbus::MethodCall* method_call,
                        dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    DCHECK(delegate_);

    dbus::MessageReader reader(method_call);
    dbus::ObjectPath transport_path;
    dbus::MessageReader properties(method_call);
    if (!reader.PopObjectPath(&transport_path) ||
        !reader.PopArray(&properties)) {
      LOG(WARNING) << "SetConfiguration called with incorrect parameters: "
                   << method_call->ToString();
      return;
    }

    delegate_->SetConfiguration(transport_path, properties);

    response_sender.Run(dbus::Response::FromMethodCall(method_call));
  }

  // Called by dbus:: when the remote device receives the configuration for
  // media transport.
  void SelectConfiguration(
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    DCHECK(delegate_);

    dbus::MessageReader reader(method_call);
    const uint8_t* capabilities = nullptr;
    size_t length = 0;
    if (!reader.PopArrayOfBytes(&capabilities, &length)) {
      LOG(WARNING) << "SelectConfiguration called with incorrect parameters: "
                   << method_call->ToString();
      return;
    }

    std::vector<uint8_t> configuration(capabilities, capabilities + length);

    // |delegate_| generates the response to |SelectConfiguration| and sends it
    // back via |callback|.
    Delegate::SelectConfigurationCallback callback = base::Bind(
        &BluetoothMediaEndpointServiceProviderImpl::OnConfiguration,
        weak_ptr_factory_.GetWeakPtr(),
        method_call,
        response_sender);

    delegate_->SelectConfiguration(configuration, callback);
  }

  // Called by dbus:: when the remote device is about to close the connection.
  void ClearConfiguration(
      dbus::MethodCall* method_call,
      dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    DCHECK(delegate_);

    dbus::MessageReader reader(method_call);
    dbus::ObjectPath transport_path;
    if (!reader.PopObjectPath(&transport_path)) {
      LOG(WARNING) << "ClearConfiguration called with incorrect parameters: "
                   << method_call->ToString();
      return;
    }

    delegate_->ClearConfiguration(transport_path);

    response_sender.Run(dbus::Response::FromMethodCall(method_call));
  }

  // Called by Bluetooth daemon to do the clean up after unregistering the Media
  // Endpoint.
  void Release(dbus::MethodCall* method_call,
               dbus::ExportedObject::ResponseSender response_sender) {
    DCHECK(OnOriginThread());
    DCHECK(delegate_);

    delegate_->Release();

    response_sender.Run(dbus::Response::FromMethodCall(method_call));
  }

  // Called by Delegate to response to a method requiring transport
  // configuration.
  void OnConfiguration(dbus::MethodCall* method_call,
                       dbus::ExportedObject::ResponseSender response_sender,
                       const std::vector<uint8_t>& configuration) {
    DCHECK(OnOriginThread());

    // Generates the response to the method call.
    scoped_ptr<dbus::Response> response(
        dbus::Response::FromMethodCall(method_call));
    dbus::MessageWriter writer(response.get());
    if (configuration.empty()) {
      LOG(WARNING) << "OnConfiguration called with empty configuration.";
      writer.AppendArrayOfBytes(nullptr, 0);
    } else {
      writer.AppendArrayOfBytes(&configuration[0], configuration.size());
    }
    response_sender.Run(response.Pass());
  }

  // Origin thread (i.e. the UI thread in production).
  base::PlatformThreadId origin_thread_id_;

  // D-Bus Bus object is exported on.
  dbus::Bus* bus_;

  // All incoming method calls are passed on to |delegate_|. |callback| passed
  // to |delegate+| will generate the response for those methods whose returns
  // are non-void.
  Delegate* delegate_;

  // D-Bus object path of the object we are exporting, kept so we can unregister
  // again in you destructor.
  dbus::ObjectPath object_path_;

  // D-Bus object we are exporting, owned by this object.
  scoped_refptr<dbus::ExportedObject> exported_object_;

  // Weak pointer factory for generating 'this' printers that might live longer
  // than we do.
  // Note This should remain the last member so it'll be destroyed and
  // invalidate it's weak pointers before any other members are destroyed.
  base::WeakPtrFactory<BluetoothMediaEndpointServiceProviderImpl>
      weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BluetoothMediaEndpointServiceProviderImpl);
};

BluetoothMediaEndpointServiceProvider::BluetoothMediaEndpointServiceProvider() {
}

BluetoothMediaEndpointServiceProvider::~BluetoothMediaEndpointServiceProvider()
{}

BluetoothMediaEndpointServiceProvider*
    BluetoothMediaEndpointServiceProvider::Create(
    dbus::Bus* bus,
    const dbus::ObjectPath& object_path,
    Delegate* delegate) {
  // Returns a real implementation.
  if (!DBusThreadManager::Get()->IsUsingStub(DBusClientBundle::BLUETOOTH)) {
    return new BluetoothMediaEndpointServiceProviderImpl(
        bus, object_path, delegate);
  }
  // Returns a fake implementation.
  return new FakeBluetoothMediaEndpointServiceProvider(object_path, delegate);
}

}  // namespace chromeos
