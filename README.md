# Tarefa U7.C1.Final

Esta é a tarefa final da primeira fase do EmbarcaTech. Ela envolveu a
criação de um projeto original que utilizasse a placa BitDogLab e os
assuntos aprendidos ao longo da fase. Com isso eu então criei o ROTUNE,
que consiste em um programa, rodando na placa, que utiliza das entradas
(Joystick e botões) para o controle do módulo PWM conectado a um dos
buzzers, basicamente tornando a placa em um mini-instrumento.

## Execução

Para rodar este código, é possível utilizar o CMake para a compilação,
enviando depois o arquivo `.uf2` para a placa via USB. Com isso, o
programa irá rodar na placa, tendo então as opções seguintes para
interação:

- Manipular os eixos X e Y do joystick para escolher uma nota (começando
    na direita, em sentido anti-horário, é possível escolher uma de 12
    notas em uma oitava);

- Apertar (e segurar) o botão B para tocar a nota (é necessário segurar
    o joystick enquanto isso acontece);

- Apertar o botão A para subir uma oitava;

- Apertar o botão do joystick para descer uma oitava;

Durante a execução do programa também é possível ver na tela (display
OLED SSD1306 da placa) informações sobre a execução, em especial a nota
tocando e o ID da nota, que pode variar entre 0 (C-2) e 95 (B-10).
