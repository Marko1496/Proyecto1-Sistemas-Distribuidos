-module(server).
-compile(export_all).


punto()->{rand:uniform(), rand:uniform()}.

estaDentro({X,Y})-> (X*X+Y*Y) =< 1.

suma(false)->0;
suma(true)->1.

cuentaPuntos(0,D,T,Server)-> Server ! {D,T};
cuentaPuntos(N,D,T,Server)-> Pto = punto(),
    cuentaPuntos(N-1, D+suma(estaDentro(Pto)), T+1, Server).

generaHilos(0, _Server)->ok;
generaHilos(N,Server)->spawn(fun()->cuentaPuntos(1000, 0, 0, Server) end), generaHilos(N-1,Server).

piServer(D,T)->
    receive
        {Dentro,Total} -> piServer(D+Dentro, T+Total);
        pi -> io:format("~p~n", [D*4/T]),
                piServer(D,T);
        cumbia->io:format("Me muero~n", []);
        X -> io:format("recibo: ~p~n",[X]), piServer(D,T)
    end.

% MiServer = spawn (fun()->server:piServer(0,0)end).

% server:generaHilos(1000, MiServer).
% MiServer ! pi.
