-module(huffman).

%% API
-export([encode/1, decode/2]).
-export([profile/1]).

%%--------------------------------------------------------------------------
%% API
%%--------------------------------------------------------------------------

encode(Text)  ->
    Tree  = tree(freq_table(Text)),
    Dict = dict:from_list(codewords(Tree)),
    Code = << <<(dict:fetch(Char, Dict))/bitstring>> || Char <- Text >>,
    {Code, Tree}.

decode(Code, Tree) ->
    decode(Code, Tree, Tree, []).

profile(OutFile) ->
    {ok, Binary} = file:read_file("priv/lipsum.txt"),
    Text = binary_to_list(Binary),
    fprof:trace(start),
    {Code, Tree} = encode(Text),
    decode(Code, Tree),
    fprof:trace(stop),
    fprof:profile(),
    fprof:analyse({dest, OutFile}).

%%--------------------------------------------------------------------------
%% Internal functions
%%--------------------------------------------------------------------------

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
