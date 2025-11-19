const botao = document.getElementById('botao');
const valor = document.getElementById('valor');
const barraProgresso = document.getElementById('barra');
const socket = io('http://localhost:3000');


const textoOriginal = botao.textContent;

botao.addEventListener("click", () => {
    
    if (botao.textContent === textoOriginal) {
        botao.textContent = "fechar a janela";  
        socket.emit('botao_clicado', { acao: 'A' }); // A == abrir
    } else {
        botao.textContent = textoOriginal;
        socket.emit('botao_clicado', { acao: 'F' }); // F == fechar
    }
})

// Receber dados de luminosidade do socket
socket.on('luminosidade', (data) => {
    const valorAtual = data.valor;
    valor.textContent = valorAtual + '%';

    if (valorAtual <= 10) {
        barraProgresso.style.backgroundColor = 'red';
    } 
    else if (valorAtual <= 30) {
        barraProgresso.style.backgroundColor = 'orange';
    } 
    else if (valorAtual <= 50) {
        barraProgresso.style.backgroundColor = 'yellow';
    } 
    else if (50 < valorAtual > 90){
        barraProgresso.style.backgroundColor = 'green';
    }
    else  {
        barraProgresso.style.backgroundColor = 'red';
    } 
    barraProgresso.style.width = valorAtual + '%';
});