-module(huffman).

-export([encode/1, decode/2, readlines/1, comprimir/1, descomprimir/2, main/0]).

encode(Text)  ->
    Tree  = tree(freq_table(Text)),
    Dict = dict:from_list(codewords(Tree)),
    Code = << <<(dict:fetch(Char, Dict))/bitstring>> || Char <- Text >>,
    {Code, Tree}.

decode(Code, Tree) ->
    decode(Code, Tree, Tree, []).

main() ->
    {Code, Tree, Dict} = encode("this is an example for huffman encoding"),
    [begin
        io:format("~s: ",[[Key]]),
        print_bits(Value)
     end || {Key, Value} <- lists:sort(dict:to_list(Dict))],
    io:format("encoded: "),
    print_bits(Code),
    io:format("decoded: "),
    io:format("~s\n",[decode(Code, Tree)]).

decode(<<>>, _, _, Result) ->
    lists:reverse(Result);
decode(<<0:1, Rest/bits>>, Tree, {L = {_, _}, _R}, Result) ->
    decode(<<Rest/bits>>, Tree, L, Result);
decode(<<0:1, Rest/bits>>, Tree, {L, _R}, Result) ->
    decode(<<Rest/bits>>, Tree, Tree, [L | Result]);
decode(<<1:1, Rest/bits>>, Tree, {_L, R = {_, _}}, Result) ->
    decode(<<Rest/bits>>, Tree, R, Result);
decode(<<1:1, Rest/bits>>, Tree, {_L, R}, Result) ->
    decode(<<Rest/bits>>, Tree, Tree, [R | Result]).

codewords({L, R}) ->
    codewords(L, <<0:1>>) ++ codewords(R, <<1:1>>).

codewords({L, R}, <<Bits/bits>>) ->
    codewords(L, <<Bits/bits, 0:1>>) ++ codewords(R, <<Bits/bits, 1:1>>);
codewords(Symbol, <<Bits/bitstring>>) ->
    [{Symbol, Bits}].

tree([{N, _} | []]) ->
    N;
tree(Ns) ->
    [{N1, C1}, {N2, C2} | Rest] = lists:keysort(2, Ns),
    tree([{{N1, N2}, C1 + C2} | Rest]).

freq_table(Text) ->
    freq_table(lists:sort(Text), []).

freq_table([], Acc) ->
    Acc;
freq_table([S | Rest], Acc) ->
    {Block, MoreBlocks} = lists:splitwith(fun (X) -> X == S end, Rest),
    freq_table(MoreBlocks, [{S, 1 + length(Block)} | Acc]).

print_bits(<<>>) ->
  io:format("\n");
print_bits(<<Bit:1, Rest/bitstring>>) ->
  io:format("~w", [Bit]),
  print_bits(Rest).


%------------------------------------------ Leer Archivo

readlines(FileName) ->
    {ok, Device} = file:open(FileName, [read]),
    try get_all_lines(Device)
      after file:close(Device)
    end.

get_all_lines(Device) ->
    case io:get_line(Device, "") of
        eof  -> [];
        Line -> Line ++ get_all_lines(Device)
    end.

%--------------------------------------------------

comprimir(FileName) -> Contenido = readlines(FileName),
                       Codigo = encode(Contenido),
                       Binario = term_to_binary(Codigo),
                       {ok, IO} = file:open(string:concat(FileName, ".mrkb"), [raw, write, binary]),
                       file:pwrite(IO,0,Binario),
                       file:close(IO).

descomprimir(FileName, Nombre) -> {ok, A} = file:read_file(FileName),
                                   {B, C} = binary_to_term(A),
                                   {ok, IO} = file:open(Nombre, write),
                                   file:write(IO, decode(B, C)),
                                   file:close(A),
                                   file:close(IO).
