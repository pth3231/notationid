main:
	cl .\src\main.cpp /Fe: main.exe /Zi /fsanitize=address
	move main.exe .\bin
	move main.obj .\bin
	move main.pdb .\bin
	move vc140.pdb .\bin
	.\bin\main.exe

clean:
	del .\bin\main.exe .\bin\main.obj .\bin\vc140.pdb .\bin\main.pdb