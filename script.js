//DOM Elements
const startScreen = document.getElementById("start-screen");
const quizScreen = document.getElementById("quiz-screen");
const resultScreen = document.getElementById("result-screen");
const startButton = document.getElementById("start-btn");
const questionText = document.getElementById("question-text");
const answersContainer = document.getElementById("answers-container");
const currentQuestionSpan = document.getElementById("current-question");
const totalQuestionsSpan = document.getElementById("total-questions");
const scoreSpan = document.getElementById("score");
const finalScoreSpan = document.getElementById("final-score");
const maxScoreSpan = document.getElementById("max-score");
const resultMessage = document.getElementById("result-message");
const restartButton = document.getElementById("restart-btn");
const progressBar = document.getElementById("progress");
let questionImage = document.getElementById("question-image");
//Array of quiz questions(objects)
const quizQuestions = [
    {
        question: "What is Flav's real name?",
        answers: [   //An array of objects
            { text: "William Drayton", correct: true },
            { text: "Marcus Jackson", correct: false },
            { text: "Darryl Thompson", correct: false },
            { text: "Kevin Robinson", correct: false }
        ],
        image: "FlavourFlav.jpg"
    },
    {
        question: "Who won season 1 of Flavour of Love?",
        answers: [
            { text: "Deelishis (Chandra Davis)", correct: false },
            { text: "New York (Tiffany Pollard)", correct: false },
            { text: "Pumpkin (Brooke Thompson)", correct: false },
            { text: "Hoopz (Nicole Alexander)", correct: true },
        ],
        image: "q2.jpg"
    },
    {
        question: "Who of the following was the main snitch in the house(Season 1)?",
        answers: [
            { text: "Hottie", correct: false },
            { text: "Red Oyster", correct: true },
            { text: "Pumpkin", correct: false },
            { text: "Serious", correct: false },
        ],
        image: "q3.jpg"
    },
    {
        question: "What was the name of Flav's butler?",
        answers: [
            { text: "Little Anthony", correct: false },
            { text: "Gerald D.", correct: false },
            { text: "Big Rick", correct: true },
            { text: "B Man", correct: false },
        ],
        image: "q4.jpg"
    },
    {
        question: "Where did Flav take the final two ladies(Season 1)?",
        answers: [
            { text: "Cuba", correct: false },
            { text: "Guatemala", correct: false },
            { text: "France", correct: false },
            { text: "Mexico", correct: true },
        ],
        image: "q5.jpg"
    },
    {
        question: "Who had a fight on the first episode(Season 2) and were kicked off?",
        answers: [
            { text: "Choclate and Eyez", correct: false },
            { text: "Hood and Wire", correct: false },
            { text: "Saaphyri and H-Town", correct: true },
            { text: "Bamma and Wire", correct: false },
        ],
        image: "q6.jpg"
    },
    {
        question: "Who wanted to go back to China and get a Chinese man(Season 2)?",
        answers: [
            { text: "Tiger", correct: false },
            { text: "Bootz", correct: false },
            { text: "Payshintz", correct: true },
            { text: "Wire", correct: false },
        ],
        image: "q7.jpg"
    },
];

//Quiz State Vars
let currentQuestionIndex = 0;
let score = 0;
let answerDisabled = false;

totalQuestionsSpan.textContent = quizQuestions.length;
maxScoreSpan.textContent = quizQuestions.length;

//event listeners
startButton.addEventListener("click", startQuiz);
restartButton.addEventListener("click", restartQuiz);

function startQuiz(){
    console.log("quiz started");
    //reset vars
    currentQuestionIndex = 0;
    score = 0;
    scoreSpan.textContent = 0;
    startScreen.classList.remove("active");
    quizScreen.classList.add("active");
    showQuestion();
}

function showQuestion(){
    //reset state
    answerDisabled = false;
    questionImage.style.display = "none";

    const currentQuestion = quizQuestions[currentQuestionIndex];

    currentQuestionSpan.textContent = currentQuestionIndex + 1;

    const progressPercent = (currentQuestionIndex / quizQuestions.length) * 100;
    progressBar.style.width = progressPercent + "%";
    questionText.textContent = currentQuestion.question;
    questionImage.src = currentQuestion.image;

    answersContainer.innerHTML = "";    //clear content
    currentQuestion.answers.forEach(answer => {
        const button = document.createElement("button");
        button.textContent = answer.text;
        button.classList.add("answer-btn");

        //What is dataset - property of button element that allows you to store custom data
        button.dataset.correct = answer.correct;

        button.addEventListener("click", selectAnswer);
        answersContainer.appendChild(button);
    });
}

function selectAnswer(event){
    //Optimization check
    if(answerDisabled) return;

    answerDisabled = true;
    questionImage.style.display = "block";

    const selectedButton = event.target;
    const isCorrect = selectedButton.dataset.correct === "true";

    Array.from(answersContainer.children).forEach(button => {
        if(button.dataset.correct === "true"){
            button.classList.add("correct")
        }else if (button === selectedButton){
            button.classList.add("incorrect");
        }
    });

    if(isCorrect){
        score++;
        scoreSpan.textContent = score;
    }

    setTimeout(() => {
        currentQuestionIndex++;

        //Check if there are more questions
        if(currentQuestionIndex < quizQuestions.length){
            showQuestion()
        }else{
            showResults();
        }
    },4000)
}

function showResults(){
    quizScreen.classList.remove("active");
    resultScreen.classList.add("active");

    finalScoreSpan.textContent = score;

    const percentage = (score/quizQuestions.length) * 100;
    if(percentage === 100){
        resultMessage.textcontent = "Perfect Score, Genius!";
    }else if(percentage >= 80){
        resultMessage.textContent = "Great job! You're good at this!";
    }else if(percentage >= 60){
        resultMessage.textContent = "Decent effort, keep studying.";
    }else if(percentage >= 40){
        resultMessage.textContent = "Eish, Try again to improve.";
    }else{
        resultMessage.textContent = "Have you considered studying?";
    }


}

function restartQuiz(){
    console.log("quiz restarted")
    resultScreen.classList.remove("active");

    startQuiz();
}