const botao = document.getElementById('botao');
const valor = document.getElementById('valor');
const barraProgresso = document.getElementById('barra');
const socket = io('http://localhost:3000');


const textoOriginal = botao.textContent;

botao.addEventListener("click", () => {
    
    if (botao.textContent === textoOriginal) {
        botao.textContent = "fechar a janela";  
        socket.emit('botao_clicado', { acao: 'abrir' });
    } else {
        botao.textContent = textoOriginal;
        socket.emit('botao_clicado', { acao: 'fechar' });
    }
})

// setInterval(() => {
//     // Substituir valor atual pelo oq vem do socket
//     const valorAtual = Math.floor(Math.random() * 101);
//     valor.textContent = valorAtual + '%';

//     if (valorAtual < 20) {
//         barraProgresso.style.backgroundColor = 'red';
//     } else if (valorAtual < 40) {
//         barraProgresso.style.backgroundColor = 'orange';
//     } else if (valorAtual < 80) {
//         barraProgresso.style.backgroundColor = 'yellow';
//     } else {
//         barraProgresso.style.backgroundColor = 'green';
//     }
//     barraProgresso.style.width = valorAtual + '%';
// }, 2000);

// Receber dados de luminosidade do socket
socket.on('luminosidade', (data) => {
    const valorAtual = data.valor;
    valor.textContent = valorAtual + '%';

    if (valorAtual < 20) {
        barraProgresso.style.backgroundColor = 'red';
    } else if (valorAtual < 40) {
        barraProgresso.style.backgroundColor = 'orange';
    } else if (valorAtual < 80) {
        barraProgresso.style.backgroundColor = 'yellow';
    } else {
        barraProgresso.style.backgroundColor = 'green';
    }
    barraProgresso.style.width = valorAtual + '%';
});