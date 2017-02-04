#!/bin/ruby
# Generates classes for all the different requests

require_relative '../../Helpers/CommonCode'
require_relative '../../Helpers/FileGen'

abort "no target file provided" if ARGV.count < 1

generator = Generator.new ARGV[0]

thingsToGenerate = [

  ["Connect",
   [
     { type: "int32_t", name: "CheckValue", default: "42" }
   ]],

   ["Security",
   [
     { type: "CONNECTION_ENCRYPTION", as: "int32_t", name: "SecureType" },
     { type: "std::string", name: "PublicKey", default: "" },
     { type: "std::string", name: "AdditionalSettings", default: "" }
   ]],
   
   ["Authenticate",
   [
     { type: "std::string", name: "UserName", default: "" },
     { type: "uint64_t", as: "sf::Uint64", name: "AuthToken", default: "0" },
     { type: "std::string", name: "AuthPasswd", default: "" }
   ]],

  ["Identification",
   [
     { type: "std::string", name: "DDOSBlock", default: "\"#{"v" * 65}\"" }
   ]],

  ["RemoteConsoleOpen",
   [
     { type: "int32_t", name: "SessionToken" }
   ]],

  ["RemoteConsoleAccess",
   [
     { type: "int32_t", name: "SessionToken" }
   ]],

  ["JoinServer",
   [
     { type: "int32_t", name: "MasterServerToken" }
   ]],

  ["GetSingleSyncValue",
   [
     { type: "std::string", name: "NameOfValue" }
   ]],

  ["RequestCommandExecution",
   [
     { type: "std::string", name: "Command" }
   ]],

  ["ConnectInput",
   [
     { type: "sf::Packet", name: "DataForObject", move: true }
   ]],

  ["WorldClockSync",
   [
     { type: "int32_t", name: "WorldID" },
     { type: "int32_t", name: "Ticks" },
     { type: "int32_t", name: "EngineMSTweak" },
     { type: "bool", name: "Absolute" }
   ]],

   ["DoRemoteConsoleOpen",
   [
     { type: "int32_t", name: "Token" }
   ]]
   
]

# Add all classes
thingsToGenerate.each do |type|

  created = ResponseClass.new("Request#{type[0]}")
  
  created.base("NetworkRequest")
  created.addDeserializeArg("uint32_t idforresponse")
  created.deserializeBase("idforresponse") 
  created.baseConstructor("NETWORK_REQUEST_TYPE::#{type[0]}")
  
  # Parameters
  type[1].each do |n|

    # n is a hash of the arguments
    created.addMember n
  end

  generator.add created
end


# Output the file
generator.run
