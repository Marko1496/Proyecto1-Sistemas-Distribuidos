-module(proyecto).

-compile(export_all).

encode(Text, Tree)  ->
    Dict = dict:from_list(codewords(Tree)),
    Code = << <<(dict:fetch(Char, Dict))/bitstring>> || Char <- Text >>,
    Code.

decode(Code, Tree) ->
    decode(Code, Tree, Tree, []).

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

%-------------------------------------------------- Comprimir y descomprimir

comprimir(FileName, Datos) -> Binario = term_to_binary(Datos),
                              {ok, IO} = file:open(string:concat(FileName, ".mrkb"), [raw, write, binary]),
                              file:pwrite(IO,0,Binario),
                              file:close(IO).

descomprimir(FileName, Nombre) -> {ok, A} = file:read_file(FileName),
                                   {FreqT, Datos} = binary_to_term(A),
                                   escribirArchivo(Nombre, FreqT, Datos).

escribirArchivo(_Nombre, _FreqT, []) -> ok;
escribirArchivo(Nombre, FreqT, [H|T]) -> file:write_file(Nombre, decode(H, tree(FreqT)), [append]),
                                          escribirArchivo(Nombre, FreqT, T).


%-------------------------------------------------- Servidor
miembro(_E, [])->false;
miembro({E, _F},[{E, _X}|_T])->true;
miembro({E, F},[_H|T])->miembro({E, F},T).

sumarP(A, B) -> sumarP(A, B, []).
sumarP(_E, [], B)->B;
sumarP({E, F},[{E, X}|T], L)->L++[{E, F+X}]++T;
sumarP({E, F},[H|T],L)->sumarP({E, F},T, L++[H]).

sumFreq(A, []) -> A;
sumFreq(A, [H|T]) -> case miembro(H, A) of true -> sumFreq(sumarP(H, A), T); false -> sumFreq(A++[H], T) end.

  dividirA(Archivo) -> Tamano = filelib:file_size(Archivo), List = readlines(Archivo),
                       if (Tamano < 101) ->
                         [ lists:sublist(List, X, (Tamano div 2)+1) || X <- lists:seq(1,length(List),(Tamano div 2)+1) ];
                         (Tamano < 1001) ->
                         [ lists:sublist(List, X, (Tamano div 4)+1) || X <- lists:seq(1,length(List),(Tamano div 4)+1) ];
                       (Tamano > 1000) ->
                         [ lists:sublist(List, X, (Tamano div 8)+1) || X <- lists:seq(1,length(List),(Tamano div 8)+1) ]
                       end.



servidorP(FreqT, Cant, Hilos, Partes, File) ->
  receive
      {[{A,B}|T], Hilo} -> if (Cant =:= 1) -> F = sumFreq(FreqT, [{A,B}|T]), broadcast(Hilos++[Hilo], tree(F)), servidorP(F, length(Hilos++[Hilo]), Hilos, Partes, File);
                              (Cant =/= 1) -> X = sumFreq(FreqT, [{A,B}|T]), servidorP(X, Cant-1, Hilos++[Hilo], Partes, File) end;

      {Num, Codigo} -> if (Cant =:= 1) -> comprimir(File, {FreqT ,listaCodigos(lists:sort(Partes++[{Num, Codigo}]))});
                          (Cant =/= 1) -> servidorP(FreqT, Cant-1, Hilos, Partes++[{Num, Codigo}], File) end;


      Archivo -> APartes = dividirA(Archivo), generarHilos(self(), APartes), servidorP(FreqT, length(APartes), [], Partes, Archivo)
  end.

broadcast([], _Mensaje) -> ok;
broadcast([H|T], Mensaje) -> H ! Mensaje, broadcast(T, Mensaje).

comParte(Servidor, Texto, Num)-> Servidor ! {freq_table(Texto), self()},
  receive
    Arbol -> Servidor ! {Num, encode(Texto, Arbol)}
  end.

generarHilos(A, B) -> generarHilos(A, B ,0).
generarHilos(_, [], _) -> ok;
generarHilos(Servidor, [H| T], C) -> spawn(fun()->comParte(Servidor, H, C+1) end), generarHilos(Servidor, T, C+1).

listaCodigos(L) -> listaCodigos(L, []).
listaCodigos([], Acum) -> Acum;
listaCodigos([{_Num, Cod}|T], Acum) -> listaCodigos(T, Acum++[Cod]).


% MiServer = spawn (fun()->huffman:servidorP([],"hola",[],[], "")end).
% MiServer ! "texto.txt".
