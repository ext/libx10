-module(erlx10).
-include("defines.hrl").
-export([
	start/0,
    start/1,
    stop/1,
	create_dimmer/2,
    create_relay/2,
    
    dimmer_set_abs/2
]).

create_dimmer(Port, Address) ->
	Command = lists:flatten([?CMD_DEVICE_CREATE, ?DEVICE_TYPE_DIMMER, Address]),
    create_device(Port, dimmer, Command).

create_relay(Port, Address) ->
	Command = lists:flatten([?CMD_DEVICE_CREATE, ?DEVICE_TYPE_RELAY, Address]),
    create_device(Port, relay,  Command).

create_device(Port, Type, Command) ->
	case command(Port, Command) of
        {ok, [DeviceID]} ->
            {x10_device, Type, DeviceID};
    	Error ->
            Error
	end.

dimmer_set_abs(Device, Value) ->
    

command(Port, Command) ->
	port_command(Port, Command),
    receive
         {Port, {data, Data}} when is_list(Data) ->
             case Data of
             	[0|Result] ->
					{ok, Result};
			 	[N|Result] ->
					{error, N, Result}
             end
    end.

start() ->
    proc_lib:start_link(?MODULE, start, [self()], 5000, []).

start(Parent) ->
	case code:where_is_file("erlx10.so") of
		non_existing ->
			error_logger:error_report("Could not find erlx10.so.so"),
            proc_lib:init_ack(Parent, {error, erlx10_not_loaded}),
			exit({error, erlx10_not_loaded});
       Fullpath ->
           Path = filename:dirname(Fullpath),
           
           case erl_ddll:load_driver(Path, "erlx10") of
                ok -> ok;
                {error, already_loaded} -> ok;
                {error, Error} ->
                	error_logger:error_report(["Could not load erlx10 driver.", {reason, erl_ddll:format_error(Error)}]),
                    proc_lib:init_ack(Parent, {error, erlx10_not_loaded}),
                    exit({error, erlx10_not_loaded})
           end,
           
           Port = open_port({spawn, "erlx10"}, []),
           io:format("~p~n", [Port]),
           Dim = create_dimmer(Port, "A2"),
			io:format("dim1: ~p~n", [Dim]),
    	   Dim2 = create_relay(Port, "A3"),
			io:format("dim2: ~p~n", [Dim2])

	end,
    
    init:stop(0).

stop(_Port) ->
    void.
