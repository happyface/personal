// Erica Hollis, John Lipnicki, Amritbir Bhullar
//Final Project
// 12/9/08
//This is a hangman game that allows one user to enter a word 
//and another user is allowed 6 wrong tries.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cctype>

using namespace std;

const int MAX = 16; // The maxium amount of characters in the users word
const int LET = 10; // In case the user types in more than one letter.
const int WRONG = 20; // Constant for the wrongGuesses character \array.
int i, NUMGUESSES = 0; 
void correct(char letter[], char word[], bool &y, char alreadyG[]);
//Checks if the the user has guessed a correct Letter
void check(char letter[], int sLength, char word[], char gWord[]);
//displays what the user has unlocked and ends the program if they've guessed the word
void gallows(int &life, char word[]);
// outputs the pictures of the gallows depending on how the player is doing

void main(void)
{
	char secret[MAX], copy[MAX], alreadyGuessed[MAX], wrongGuess[WRONG] = " ";
	char guess[LET];
	int num=7, end = 1, stringlength;
	bool x=1;
	char userknow[]="_________________";//what the user knows
	cout<<"Welcome to Capital Punishment, a hangman game!";
	cout<<endl<<endl;
	cout<<"Please enter the word to be guessed. (15 letters max): ";
	cin.getline(secret, MAX);
	system("cls"); // Clears the screen.
	for (i=0; i<MAX; i++){
		secret[i]=tolower(secret[i]);//makes the Program case-insensative
		copy[i] = secret[i]; // Makes a copy of the word.
	}
	stringlength = strlen(secret);
	cout<<"Now we'll try to guess the word, one letter at a time!\n";
	cout << "You are allowed 6 wrong guesses.\n";
	do{
		NUMGUESSES++;
		cout << "\nPlease enter your guess: " ;
		cin.getline(guess, LET);
		guess[0]=tolower(guess[0]); // Makes the users input case insensative.
		system("cls");
		if (strlen(guess)>1){ // Check if the user enters more than one letter.
			cout << "Please enter only one letter.\n";
			continue;
		}
		correct(guess, copy, x, alreadyGuessed); // See if the guess is correct.
		if (!x){
			strcat (wrongGuess, guess); // Adds the wrong guesses into a character array.
			strcat (wrongGuess, " ");
			cout << "Wrong guess.\n\nYour wrong guesses so far are:" <<wrongGuess << "\n\n";
			num--; // Takes a point away from the little mans life.
		}
		if (NUMGUESSES>1 && x && num <7){ // If the guess is correct, still show the wrong guesses.
			cout << "Your wrong guesses so far are:" << wrongGuess << endl << endl;
		}
		check(guess, stringlength, copy, userknow);
		gallows(num, secret); // Outputs the execution.
	}while(num>0);
}

void correct(char letter[], char word[], bool &y, char alreadyG[])
// Checks if the letter was already entered. If not, checks if the the user has entered a correct letter.											
{
	y=0;
	for (i=0;i<MAX; i++){
		if (letter[0] == alreadyG[i]){ // Checks to see if the letter has already been guessed.
			cout << "You already guessed that letter.\n\n";
			y++;
			break;
		}
		else if (letter[0] == word[i]){ // Checks if the guess is correct.
			cout << "Good guess!\n\n";
			y++;
			break;
		}
	}
	for (i=0;i<MAX; i++)
			alreadyG[i] = letter[0]; // Assigns the guessed letter into a character array of the letters guessed so far.
}

void check(char letter[], int sLength, char word[], char gWord[])//displays what the user has unlocked and
																//ends the program if they've guessed the word.
{
	for (i=0; i<sLength; i++)//displays how many letters the user has unlocked
	{
		if (letter[0] == word[i])
			gWord[i] = word[i];
		cout << gWord[i];
		cout << " ";
	}
	int complete=0;
	for (i=0; i<sLength; i++)//this checks and sees if the user has the full word discovered
	{
		if (gWord[i] != word[i])
			complete++;
	}
	if(complete==0){//if the user has guessed the word the program ends
		cout<<"\n\nYou win!!\n\n";
		exit(1);
	}
	cout << endl;
}
 
void gallows(int &life, char letter[]) // is sent a number and outputs the hanging
{
	if(life==6){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || "<<endl
			<<" || "<<endl
			<<"[II] "<<endl<<endl<<endl;
	}
	else if(life==5){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || O "<<endl
			<<" || "<<endl
			<<"[II] "<<endl<<endl;
	}
	else if(life==4){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || O "<<endl
			<<" || | "<<endl
			<<"[II] "<<endl<<endl;
	}
	else if(life==3){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || 0 "<<endl
			<<" || | "<<endl
			<<" ||/ "<<endl
			<<"[II] "<<endl<<endl;
	}
	else if(life==2){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || O"<<endl
			<<" || | "<<endl
			<<" ||/ L "<<endl
			<<"[II] "<<endl<<endl;
	}
	else if(life==1){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || O"<<endl
			<<" || |- "<<endl
			<<" || / L "<<endl
			<<"[II] "<<endl<<endl;
	}
	else if(life==0){
		cout<<endl<<endl<<endl
			<<" ____ "<<endl
			<<" / | "<<endl
			<<" || O"<<endl
			<<" ||-|- "<<endl
			<<" ||/ L "<<endl
			<<"[II] "<<endl;
		cout << "You lose!" << endl;
		cout << endl <<"The word was " << letter <<"!\n\n";
		exit(1);
	}
}
