"use strict";
//        VARIABILI 'GLOBALI'  ///////////////////

const TILES = 9;

// same as TILES_IN_A_COLUMN
const TILES_IN_A_ROW = Math.sqrt(TILES);

// vettore delle posizioni
let indexes;

const cells = new Array(TILES);

const imgs = [
  "url('./img/ocean.jpg')",
  "url('./img/Doggie.jpg')",
  "url('./img/Solaire_of_Astora.png')",
  "url('./img/Sanniti.jpg')",
];

let moves = 0;

//////////////////////////////// Fine Var. GLOBALI /////////////////////////

///////////////////////////////// Funzioni di supporto /////////////////////

/**
 * this function returns a number in range [0,upperBound)
 * @param {*} upperBound
 * @returns
 */
const getRandomInt = function (upperBound) {
  return Math.floor(Math.random() * upperBound);
};

/**
 *
 * @param {*} array
 * @param {*} el
 * @returns true iff el is in array
 */
const member = function (array = [], el = null) {
  for (const e of array) {
    if (e === el) {
      return true;
    }
  }
  return false;
};

// swaps indexes in global vector indexes
const swapIndexes = function (i, j) {
  let temp = indexes[i];
  indexes[i] = indexes[j];
  indexes[j] = temp;
};

// vettore che torna la posizione corrente (all'interno del puzzle) di una casella
//di id istr (stringa: c1, c3, etc.)
const getPosIndexes = function (istr) {
  let i = Number(istr.slice(1));
  for (let j = 0; j < TILES; j++) {
    if (indexes[j] === i) {
      return j;
    }
  }
  return -1;
};

// verifica se una casella si trova sul bordo sinistro
// (prende l'indice della sua posizione corrente)
const notOnLeftEdge = function (ind) {
  let step = TILES_IN_A_ROW;
  for (let i = 0; i < step; i++) {
    if (ind === i * step) {
      return false;
    }
  }
  return true;
};

const notOnRightEdge = function (ind) {
  let step = TILES_IN_A_ROW;
  for (let i = 1; i <= step; i++) {
    if (ind === i * step - 1) {
      return false;
    }
  }
  return true;
};

const notOnTop = function (ind) {
  let step = TILES_IN_A_ROW;
  for (let i = 0; i < step; i++) {
    if (ind === i) {
      return false;
    }
  }
  return true;
};

const notOnBottom = function (ind) {
  let step = TILES_IN_A_ROW;
  // indice da cui inizia l'ultima riga...
  let init = step * (step - 1);

  for (let i = init; i < TILES; i++) {
    if (ind === i) {
      return false;
    }
  }
  return true;
};

const pickRandomImage = function () {
  return imgs[Math.floor(Math.random() * imgs.length)];
};

// Funzione di supporto che fornisce un ordinamento totalmente casuale degli indici
const scatterIndexes = function () {
  let indexes = new Array(TILES);
  let indexToassing;
  let i = 0;
  do {
    do {
      indexToassing = getRandomInt(TILES);
    } while (member(indexes, indexToassing));
    indexes[i] = indexToassing;
    i++;
  } while (i < TILES);

  return indexes;
};

//////////////////////////// fine Funzioni di Supporto ////////////////////////

// funzione eseguita al caricamento del documento html
const initialize = function () {
  // "istanzio" il puzzle...
  createCells();

  // aggiungo il listener sul bottone "nuova partita"...
  document.querySelector("#controls button").addEventListener("click", start);
  start();
};
// funzione che 'istanzia' (solo la prima volta che carico la pagina) il puzzle
const createCells = function () {
  // riferimento al div in cui piazzare il puzzle
  let game = document.getElementById("game");
  let icell;
  let txt;
  let i;
  for (i = 0; i < TILES; i++) {
    icell = document.createElement("div");
    icell.id = "c" + String(i);
    txt = document.createElement("p");
    txt.textContent = String(i);
    icell.appendChild(txt);
    cells[i] = {
      cell: icell,
      number: i,
    };
    game.appendChild(cells[i].cell);
  }
};
// Funzione per iniziare una nuova partita
const start = function () {
  // tolgo la notifica di fine gioco se necessario...
  let ending_div = document.getElementById("ending");
  if (ending_div != null && ending_div != undefined) {
    ending_div.remove();
  }

  // Aggiungo (o Ri-Aggiungo i listener sulle caselle per farle muovere)
  document.querySelectorAll("#game div").forEach(function (el) {
    el.addEventListener("click", move);
  });

  // Resetto il numero di moves
  moves = 0;
  document.querySelector("#status span").textContent = moves;
  // Rimescolo gli indici
  shuffle();
  // cambio il puzzle
  newPuzzle();
};

// Funzione che crea un nuovo puzzle
const newPuzzle = function () {
  let img = pickRandomImage();
  let game = document.getElementById("game");
  // Non funziona se cambio TILES (16-puzzle, etc.)....ma non è previsto nell'Homework (8-puzzle)
  let pos = [
    "nothing",
    "top left",
    "top center",
    "top right",
    "center left",
    "center center",
    "center right",
    "bottom left",
    "bottom center",
    "bottom right",
  ];
  // Setto le immagini
  let i;
  for (i = 1; i < TILES; i++) {
    cells[i].cell.style.backgroundImage = img;
    cells[i].cell.style.backgroundPosition = pos[i];
    cells[i].cell.style.backgroundRepeat = "no-repeat";
  }
  // aggiungo display grid al puzzle (scacchiera)
  game.classList.add("grid");
  // "Disegno" il puzzle
  makePositions();
};

// Callback associata al click sulle caselle (spostamento)
const move = function () {
  // Per capire se dovrò ridisegnare il puzzle...
  let swapped = false;
  // Trovo la posizione della casella sui cui si è cliccato sopra
  let indx = getPosIndexes(this.id);
  // Riferimento allo span contenente numero di moves
  let span = document.querySelector("#status span");

  // Se c'è una casella vuota adiacente, faccio lo swap
  if (notOnLeftEdge(indx) && indexes[indx - 1] === 0) {
    swapIndexes(indx, indx - 1);
    swapped = true;
  } else if (notOnRightEdge(indx) && indexes[indx + 1] == 0) {
    swapIndexes(indx, indx + 1);
    swapped = true;
  } else if (notOnTop(indx) && indexes[indx - Math.sqrt(TILES)] == 0) {
    swapIndexes(indx, indx - Math.sqrt(TILES));
    swapped = true;
  } else if (notOnBottom(indx) && indexes[indx + Math.sqrt(TILES)] == 0) {
    swapIndexes(indx, indx + Math.sqrt(TILES));
    swapped = true;
  }
  // console.log("Hai cliccato!:" + String(indx));
  if (swapped === true) {
    moves++;
    span.textContent = String(moves);
    makePositions();
  }
  // console.log("polarity: " + String(polarity()));
};

// Funzione che disegna le celle sullo schermo in base al vettore delle posizioni
const makePositions = function () {
  let game = document.getElementById("game");
  // vettore di stringhe da mettere nella proprietà gridTemplateAreas di game
  let strs = "";

  // Costruisco la stringa da mettere come valore della proprietà gridTemplateAreas
  // in base all'ordinamento corrente dell'array 'cells'
  // N.B.: la stringa dev'essere del tipo '"c0 c1 c2" "c3 c4 c5" "c6 c7 c8"'
  let count = 0;
  let i;
  let j;
  for (i = 0; i < TILES_IN_A_ROW; i++) {
    let str = "";
    for (j = 0; j < TILES_IN_A_ROW; j++) {
      str += "c" + String(indexes[count]);
      if (j < TILES_IN_A_ROW - 1) {
        str += " ";
      }
      count++;
    }
    if (i > 0) {
      strs += " ";
    }
    strs += '"' + str + '"';
  }

  // Riordino le celle del puzzle mediante la stringa strs costruita
  game.style.gridTemplateAreas = strs;
  verifyCompletion();
};

// Funzione che effettua lo shuffling delle posizioni
const shuffle = function () {
  let pol;

  // Ricavo una disposizione casuale degli indici a polarità pari
  do {
    indexes = scatterIndexes();
    pol = polarity();
  } while (pol % 2 != 0);
};
// Funzione per calcolare la polarità
const polarity = function () {
  let counter = 0;
  let i;
  let j;
  for (i = 0; i < TILES; i++) {
    if (indexes[i] === 0) {
      continue;
    }
    for (j = 0; j < TILES; j++) {
      if (indexes[j] === 0) {
        continue;
      }
      if (indexes[i] > indexes[j] && i < j) {
        counter++;
      }
    }
  }
  return counter;
};
// Funzione eseguita ogni volta al termine dello spostamento delle caselle.
// Tale funzione verifica se l'utente ha vinto
const verifyCompletion = function () {
  // console.log("isSolved: " + isSolved());
  if (isSolved() === true) {
    gameOver();
    // console.log("CIAO!");
  }
};
const isSolved = function () {
  let i;
  if (indexes[TILES - 1] != 0) {
    // console.log("primo fail");
    return false;
  }
  for (i = 0; i < TILES - 1; i++) {
    // console.log(String(i));
    if (indexes[i] != i + 1) {
      // console.log(String(i) + "-esimo fail");
      return false;
    }
  }
  return true;
};
// Funzione eseguita al termine della partita
const gameOver = function () {
  // Disegno la notifica di fine gioco
  let ending = document.createElement("div");
  ending.id = "ending";
  let msg = document.createElement("p");
  msg.textContent = "Bravo, Hai Vinto!";
  ending.appendChild(msg);
  document.getElementById("game").appendChild(ending);

  // Rimuovo i listener associati alle caselle fintanto che mostro la notifica di fine gioco
  document.querySelectorAll("#game div").forEach(function (el) {
    el.removeEventListener("click", move);
  });
};

window.onload = initialize;
