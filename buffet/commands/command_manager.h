// Copyright 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUFFET_COMMANDS_COMMAND_MANAGER_H_
#define BUFFET_COMMANDS_COMMAND_MANAGER_H_

#include <memory>
#include <string>

#include <base/files/file_path.h>
#include <base/macros.h>
#include <base/memory/weak_ptr.h>

#include "buffet/commands/command_dictionary.h"
#include "buffet/commands/command_queue.h"
#include "buffet/commands/dbus_command_dispatcher.h"

namespace chromeos {
namespace dbus_utils {
class ExportedObjectManager;
}  // namespace dbus_utils
}  // namespace chromeos


namespace buffet {

class CommandInstance;

// CommandManager class that will have a list of all the device command
// schemas as well as the live command queue of pending command instances
// dispatched to the device.
class CommandManager final {
 public:
  CommandManager();
  explicit CommandManager(
      const base::WeakPtr<chromeos::dbus_utils::ExportedObjectManager>&
          object_manager);

  // Get the command definitions for the device.
  const CommandDictionary& GetCommandDictionary() const;

  // Loads base/standard GCD command definitions.
  // |json| is the full JSON schema of standard GCD commands. These commands
  // are not necessarily supported by a particular device but rather
  // all the standard commands defined by GCD standard for all known/supported
  // device kinds.
  // On success, returns true. Otherwise, |error| contains additional
  // error information.
  bool LoadBaseCommands(const base::DictionaryValue& json,
                        chromeos::ErrorPtr* error);

  // Same as the overload above, but takes a path to a json file to read
  // the base command definitions from.
  bool LoadBaseCommands(const base::FilePath& json_file_path,
                        chromeos::ErrorPtr* error);

  // Loads device command schema for particular category.
  // See CommandDictionary::LoadCommands for detailed description of the
  // parameters.
  bool LoadCommands(const base::DictionaryValue& json,
                    const std::string& category,
                    chromeos::ErrorPtr* error);

  // Same as the overload above, but takes a path to a json file to read
  // the base command definitions from. Also, the command category is
  // derived from file name (without extension). So, if the path points to
  // "power_manager.json", the command category used will be "power_manager".
  bool LoadCommands(const base::FilePath& json_file_path,
                    chromeos::ErrorPtr* error);

  // Startup method to be called by buffet daemon at startup.
  // Initializes the object and loads the standard GCD command
  // dictionary as well as static vendor-provided command definitions for
  // the current device.
  void Startup();

  // Adds a new command to the command queue. Returns command ID.
  std::string AddCommand(std::unique_ptr<CommandInstance> command_instance);

 private:
  CommandDictionary base_dictionary_;  // Base/std command definitions/schemas.
  CommandDictionary dictionary_;  // Command definitions/schemas.
  CommandQueue command_queue_;
  DBusCommandDispacher command_dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(CommandManager);
};

}  // namespace buffet

#endif  // BUFFET_COMMANDS_COMMAND_MANAGER_H_
