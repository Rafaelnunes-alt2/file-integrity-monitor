# file-integrity-monitor

Monitor de integridade de arquivos com SHA-256 implementado na mão, sem OpenSSL. Gera baseline de diretório e depois detecta CHANGED, MISSING e NEW. C++17 puro com filesystem padrão, portátil Linux e Windows.

## Por que eu fiz isso

EDR detecta invasão percebendo que arquivo mudou. Quis construir esse pedaço do zero: o hash que garante identidade e a comparação que aponta diferença, sem biblioteca externa no caminho.

## Como funciona

Três partes, todas com teto.

1. Hash: SHA-256 FIPS 180-4 implementado do zero, incremental por blocos de 64 bytes, leitura em chunks de 64 KB, arquivo máximo 512 MB. Vetor oficial `abc` bate com o esperado.

2. Baseline: varre o diretório recursivo, só arquivo regular, symlink pulado para não escapar da raiz. Caminhos relativos ordenados, máximo 100000 arquivos. Salva `sha tamanho relativo` separado por tab.

3. Verify: recalcula tudo e compara com a baseline. Conta ok, changed, missing e new. Retorno 0 se idêntico, 3 se qualquer diferença.

## Segurança

Só leitura nos alvos, escrita só no arquivo de baseline indicado. Symlink nunca seguido. Path com `..` descartado na carga e na construção. DB acima de 64 MB rejeitado. Nome de arquivo com tab ou quebra de linha pulado no save. Compila com -Wall -Wextra -Wpedantic -Werror, fortify e stack protector.

## Estrutura

    src/
      main.cpp         CLI hash, init, verify e selftest
      sha256.cpp       SHA-256 FIPS do zero
      fim.cpp          varredura, save e load de baseline
    include/
      sha256.hpp       API do hash
      fim.hpp          tipos e limites
    examples/
      output-selftest.txt    vetor oficial
      output-init.txt        baseline gerada
      output-verify.txt      tamper detectado
    CMakeLists.txt
    README.md

## Como compilar

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/fim --selftest

No Windows abre o mesmo CMakeLists no Visual Studio.

## Como rodar

    fim --hash <arquivo>
    fim --init <diretório> --db <baseline.txt>
    fim --verify --db <baseline.txt> --root <diretório>
    fim --selftest

Verify retorna 0 se idêntico e 3 se há changed, missing ou new. Erro de argumento retorna 1 e falha de acesso retorna 2.

## Testes validados

    --selftest com abc igual ao vetor oficial ba7816bf — OK
    --hash de arquivo bate com sha256sum do sistema — OK
    --init em diretório com 2 arquivos gera baseline com 2 linhas — OK
    --verify limpo retorna ok igual a 2 e exit 0 — OK
    arquivo modificado detectado como CHANGED com exit 3 — OK
    arquivo apagado como MISSING e novo como NEW com exit 3 — OK

Saídas em examples/.

## Limitações

Sem modo daemon, cada verify é uma execução. Sem excluir padrão por glob, tudo regular entra. Arquivo acima de 512 MB é pulado com erro em vez de hash parcial. Permissão negada pula o arquivo sem abortar o scan.

## Próximos passos

Watch contínuo com intervalo, exclusões por glob, saída em JSON para ingestão por SIEM.

## Autor

Rafael Nunes — [@Rafaelnunes-alt2](https://github.com/Rafaelnunes-alt2)
