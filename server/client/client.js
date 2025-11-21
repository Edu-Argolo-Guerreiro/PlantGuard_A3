// Obtém referências para os elementos do HTML que serão manipulados.
const botao = document.getElementById('botao');
const valor = document.getElementById('valor');
const barraProgresso = document.getElementById('barra');

// Inicia a conexão com o servidor Socket.IO que está rodando em http://localhost:3000.
const socket = io('http://localhost:3000');

// Armazena o texto original do botão para poder restaurá-lo depois.
const textoOriginal = botao.textContent;

// Adiciona um "ouvinte" de evento de clique ao botão.
botao.addEventListener("click", () => {
    
    // Verifica se o texto do botão é o original.
    if (botao.textContent === textoOriginal) {
        // Se for, muda o texto para "fechar a janela".
        botao.textContent = "fechar a janela";  
        // E envia um evento 'botao_clicado' para o servidor com a ação 'A' (Abrir).
        socket.emit('botao_clicado', { acao: 'A' }); // A == abrir
    } else {
        // Caso contrário, restaura o texto original do botão.
        botao.textContent = textoOriginal;
        // E envia um evento 'botao_clicado' para o servidor com a ação 'F' (Fechar).
        socket.emit('botao_clicado', { acao: 'F' }); // F == fechar
    }
})

// Define o que fazer ao receber o evento 'luminosidade' do servidor.
socket.on('luminosidade', (data) => {
    // Extrai o valor da luminosidade dos dados recebidos.
    const valorAtual = data.valor;
    // Atualiza o texto do elemento 'valor' para mostrar a porcentagem atual.
    valor.textContent = valorAtual + '%';

    // Altera a cor da barra de progresso com base no valor da luminosidade.
    if (valorAtual <= 30) {
        barraProgresso.style.backgroundColor = 'red';
    } else if (valorAtual <= 50) {
        barraProgresso.style.backgroundColor = 'yellow';
    } else if (valorAtual <= 70) {
        barraProgresso.style.backgroundColor = 'green';
    } else if (valorAtual <= 90 ){
        barraProgresso.style.backgroundColor = 'yellow';
    } else { // Acima de 90
        barraProgresso.style.backgroundColor = 'red';
    } 
    // Define a largura da barra de progresso para corresponder ao valor da luminosidade.
    barraProgresso.style.width = valorAtual + '%';
});