﻿#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include"Player.h"

string gName;
int gTurn;
int gId;
int gClass;
int gTeam;
int gX;
int gY;
int globalTurn=30;
bool giveNewPlayer = false;
bool myMovement = true;
bool myAtack = false;
bool giveMovement = false;
sf::Sprite mapaSprite;
sf::Texture mapaTexture;
sf::Vector2f mapaPosition(0, 15);
std::string differentClasses[] = 
{ 
	"Paladin", 
	"Barbarian", 
	"Assasin", 
	"Archer", 
	"Shadowblade",
	"Metamothp", 
	"Monk", 
	"Mage" 
};

sf::Color classesColor[] =
{
	sf::Color(255, 255, 255),
	sf::Color(0, 160, 0),
	sf::Color(255, 255, 255),
	sf::Color(0, 160, 0),
	sf::Color(255, 255, 255),
	sf::Color(0, 160, 0),
	sf::Color(255, 255, 255),
	sf::Color(0, 160, 0)
};

std::mutex mut;

enum PlayerState
{
	Waiting,
	CharacterCreation,
	Ready,
	Playing
};

Player newPlayer;



sf::Color CheckTextColor(int peerId)
{
	sf::Color newColor = sf::Color(0, 160, 0);

	switch (peerId)
	{
		// Blue
	case 0:
		newColor = sf::Color(0, 206, 209);
		break;
		// Green
	case 1:
		newColor = sf::Color(0, 160, 0);
		break;
		// Yellow
	case 2:
		newColor = sf::Color(230, 230, 0);
		break;
		// Orange
	case 3:
		newColor = sf::Color(255, 180, 0);
		break;
		// White
	case 4:
		newColor = sf::Color(255, 255, 255);
		break;
	}

	return newColor;
}

std::string AssignPlayerName(int peerId)
{
	std::string name = "Name";

	switch (peerId)
	{
		// Blue
	case 0:
		name = "AZUL";
		break;
		// Green
	case 1:
		name = "VERDE";
		break;
		// Yellow
	case 2:
		name = "AMARILLO";
		break;
		// Orange
	case 3:
		name = "NARANJA";
		break;
	}

	return name;
}

void thread_socket_selector(std::vector<std::string>* aMsjs, std::vector<int>* msgColor, std::vector<Player>* _aPlayers, std::vector<sf::TcpSocket*> peers)
{
	sf::SocketSelector socketSelector;
	sf::Packet packet;
	sf::Socket::Status status;
	sf::String text;

	sf::String command;

	int peerId;

	for (int i = 0; i < peers.size(); i++)
		socketSelector.add(*peers[i]);


	while (socketSelector.wait())
	{
		for (int i = 0; i < peers.size(); i++)
		{
			if (socketSelector.isReady(*peers[i]))
			{
				status = peers[i]->receive(packet);

				packet >> command;

				if (command == "PLAYER")
				{
					//std::cout << "PLAYER | NEW" << std::endl;

					std::string aName;
					int aTurn;
					int aId;
					int aClass;
					int aTeam;
					int aX;
					int aY;

					packet >> aName;
					packet >> aTurn;
					packet >> aId;
					packet >> aClass;
					packet >> aTeam;
					packet >> aX;
					packet >> aY;


					gName = aName;
					gTurn=aTurn;
					gId= aId;
					gClass= aClass;
					gTeam= aTeam;
					gX= aX;
					gY= aY;
					giveNewPlayer = true;

					text = "El jugador " + aName + " ha escogido la clase " + differentClasses[aClass -1];
					peerId = 4;
				}

				else if (command == "MOVEMENT")
				{
					int aId;
					int aX;
					int aY;

					
					packet >> aId;
					packet >> aX;
					packet >> aY;


					
					gId = aId;
					
					gX = aX;
					gY = aY;
					giveMovement = true;

					

					std::cout << "MOVIMIENTO - Jugador " << i << " se ha movido ";
					text = "MOVIMIENTO - Jugador " + to_string(i) + " se ha movido";
					peerId = 4;
				}
				else if (command == "TURN")
				{
					packet >> globalTurn;

					std::cout << "El Jugador  " << i << " ha acabado el turno ";
					text = "Turno del jugador " + to_string(i) + " ha acabado";
				}
				else
				{
					std::cout << "MESSAGE | NEW" << std::endl;
					packet >> text >> peerId;
				}
				

				if (status == sf::Socket::Status::Disconnected)
				{
					socketSelector.remove(*peers[i]);
					peers[i]->disconnect();
					peers.erase(peers.begin() + i);

					aMsjs->push_back("Se ha desconectado un jugador");
					msgColor->push_back(4);
				}
				else
				{
					aMsjs->push_back(text);
					msgColor->push_back(peerId);

					while (aMsjs->size() > 7)
						aMsjs->erase(aMsjs->begin() + i);
						//msgColor->erase(msgColor->begin() + i);
				}
				break;
			}
		}
	}
}

int main()
{
	Player myPlayer;
	PlayerState myState = Waiting;
	
	std::vector<std::string> aMensajes;
	std::vector<int> messageColor;

	
	sf::TcpSocket socket;
	std::vector<sf::TcpSocket*> peersVector;
	std::vector<Player> aPlayers;
	sf::TcpListener listener;
	sf::Packet packet;

	std::string ip;
	unsigned short port;
	int peers;

	sf::Socket::Status status = socket.connect("localhost", 50000, sf::milliseconds(15.f));



	if (status != sf::Socket::Done)
	{
		std::cout << "Error al establecer conexion\n";
		exit(0);
	}
	else
	{
		std::cout << "Se ha establecido conexion\n";
		socket.receive(packet);

		// Client number
		packet >> peers;
		std::cout << "Peers actuales: " << peers << std::endl;
		
		for (int i = 0; i < peers; i++)
		{
			// IP and Port
			packet >> ip >> port;


			sf::TcpSocket* aux = new sf::TcpSocket();
			status = aux->connect(ip, port);

			if (status != sf::Socket::Done)
			{
				std::cout << "Error al establecer conexion\n";
				exit(0);
			}
			else
			{
				std::cout << "Conectado con >> IP: " << ip << " | PORT: " << port << std::endl;
				peersVector.push_back(aux);
			}
		}

		int newPort = socket.getLocalPort();

		// We don´t need the socket anymore;
		socket.disconnect();

		listener.listen(newPort);

		sf::TcpListener::Status peerStatus;
		while (peersVector.size() < 3)
		{
			// std::cout << "While | Vector Size: " << peersVector.size() << std::endl;
			sf::TcpSocket* peerSocket = new sf::TcpSocket;
			peerStatus = listener.accept(*peerSocket);

			// If its accepted then we put it in the peers vector.
			if (peerStatus == sf::TcpListener::Done)
			{
				peersVector.push_back(peerSocket);
			}
		}
	}

	std::cout << "Todos los clientes se han conectado" << std::endl;
	aMensajes.push_back("Introduce el numero de la clase a jugar");
	messageColor.push_back(4);
	std::cout << "Peer numero: " << peers << std::endl;
	
	myState = CharacterCreation;
	std::cout << "PLAYER STATE | Character creation" << std::endl;

	// PARTE GRAFICA

	sf::Vector2i screenDimensions(1000, 800);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	mapaTexture.loadFromFile("mapa2.png");
	mapaSprite.setTexture(mapaTexture);
	
	
	if (!font.loadFromFile("DroidSans.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = ">";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 14);
	text.setFillColor(CheckTextColor(peers));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 630);

	sf::Text classesText(mensaje, font, 14);
	classesText.setFillColor(sf::Color(255, 255, 255));
	classesText.setStyle(sf::Text::Bold);

	sf::Text nameText(mensaje, font, 14);
	nameText.setFillColor(sf::Color(255, 255, 255));
	nameText.setStyle(sf::Text::Bold);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 620);
	sf::RectangleShape chatRect(sf::Vector2f(800, 150));
	chatRect.setFillColor(sf::Color(100,100, 100, 160));
	chatRect.setPosition(0, 650);

	////////// Thread to receive packets////////////////////////////////////////////////
	std::thread sockSelecThread(&thread_socket_selector, &aMensajes, &messageColor, &aPlayers, peersVector);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				exit(0);
				break;
/////////////////////////////CLICK RATON /////////////////////////////////////////////////////////////////////
			case sf::Event::MouseButtonPressed:
				if (evento.mouseButton.button == sf::Mouse::Left && globalTurn==myPlayer.ID && myMovement)
				{
					int x = evento.mouseButton.x;
					int y = evento.mouseButton.y;
					if (x < 600 && y < 600) 
					{
						for (int n = 0; n <= 15; n++) {
							if (n * 40 <= x && (n + 1) * 40 > x) {
								x = n*40;
								break;
							}
						}
						for (int n = 0; n <= 15; n++) {
							if (n * 40 <= y && (n + 1) * 40 > y) {
								y = n * 40;
								break;
							}
						}
						myPlayer.move(x, y);

						sf::Packet packet;
						packet << (sf::String)"MOVEMENT";
						packet << peers << x << y;

						mensaje = "MOVIMIENTO - Me he movido";

						aMensajes.push_back(mensaje);
						messageColor.push_back(4);

						// Send the message to the rest of the clients.
						for (int i = 0; i < peersVector.size(); i++)
						{
							std::cout << "MOVEMENT | SENDING" << std::endl;
							peersVector[i]->send(packet);
						}
						if (aMensajes.size() > 7)
						{
							mut.lock();
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
							//messageColor.erase(messageColor.begin(), messageColor.begin() + 1);
							mut.unlock();
						}

						mensaje = ">";

						myMovement = false;
					}
				}
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
////////////////////////TECLA ENTER////////////////////////////////////////////////////////////////				
				else if (evento.key.code == sf::Keyboard::Return)
				{
					if (myState == CharacterCreation)
					{
						std::string classNumber = "0";
						std::string playerName = "0";

						if (mensaje.find('>') != std::string::npos)
							classNumber = (std::string)mensaje.substring(1, 1);
						else
							classNumber = (std::string)mensaje.substring(0, 1);


						myPlayer = myPlayer.setPlayer(AssignPlayerName(peers), peers+1, peers, atoi(classNumber.c_str()), peers % 2, (peers + 1)* 100, (peers + 1) * 100);

						mensaje = ">";

						sf::Packet readyPacket;
						readyPacket << (sf::String)"PLAYER";
						readyPacket << myPlayer.name;
						readyPacket << myPlayer.turno;
						readyPacket << myPlayer.ID;
						readyPacket << atoi(classNumber.c_str());
						readyPacket << myPlayer.team;
						readyPacket << myPlayer.x;
						readyPacket << myPlayer.y;

						for (int i = 0; i < peersVector.size(); i++)
							peersVector[i]->send(readyPacket);
						myState = Ready;
					}
					else if (myState == Ready)
					{
						sf::Packet packet;
						packet << (sf::String)"MESSAGE";
						packet << mensaje << peers;

						aMensajes.push_back(mensaje);
						messageColor.push_back(peers);

						// Send the message to the rest of the clients.
						for (int i = 0; i < peersVector.size(); i++)
							peersVector[i]->send(packet);

						if (aMensajes.size() > 7)
						{
							mut.lock();
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
							//messageColor.erase(messageColor.begin(), messageColor.begin() + 1);
							mut.unlock();
						}

						mensaje = ">";
					}
					else if (myState == Playing)
					{
						sf::Packet packet;
						packet << (sf::String)"MESSAGE";
						packet << mensaje << peers;

						aMensajes.push_back(mensaje);
						messageColor.push_back(peers);

						// Send the message to the rest of the clients.
						for (int i = 0; i < peersVector.size(); i++)
							peersVector[i]->send(packet);

						if (aMensajes.size() > 7)
						{
							mut.lock();
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
							//messageColor.erase(messageColor.begin(), messageColor.begin() + 1);
							mut.unlock();
						}

						mensaje = ">";
					}
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
		//separator
		if (myState == CharacterCreation) 
		{
			if (giveNewPlayer)
			{
				aPlayers.push_back(Player(gName, gTurn, gId, gClass, gTeam, gX, gY));
				giveNewPlayer = false;
			}
		}
		if (myState == Ready) 
		{
			if (giveNewPlayer)
			{
				aPlayers.push_back(Player(gName, gTurn, gId, gClass, gTeam, gX, gY));
				giveNewPlayer = false;
			}
			if (aPlayers.size() == 3) 
			{
				std::cout << "Cambia a playing " << std::endl;
				myState = Playing;
				globalTurn = 0;
			}
			
		}
		if (myState == Playing) 
		{
			if (giveMovement) 
			{
				std::cout << " Send MOVEMENT " << std::endl;

				for (int n = 0; n < aPlayers.size(); n++) 
				{
					if (aPlayers[n].ID == gId)
						aPlayers[n].move(gX, gY);
				}

				giveMovement = false;
			}

			if (!myMovement && !myAtack && globalTurn == peers)
			{
				globalTurn = (globalTurn + 1) % 4;

				sf::Packet packet;
				packet << (sf::String)"TURN";
				packet << globalTurn;

				mensaje = "Turno finalizado";

				aMensajes.push_back(mensaje);
				messageColor.push_back(4);

				// Send the message to the rest of the clients.
				for (int i = 0; i < peersVector.size(); i++)
					peersVector[i]->send(packet);

				if (aMensajes.size() > 7)
				{
					mut.lock();
					aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					//messageColor.erase(messageColor.begin(), messageColor.begin() + 1);
					mut.unlock();
				}

				mensaje = ">";

				myMovement = true;
			}
		}


		window.draw(separator);
		window.draw(chatRect);

		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setFillColor(CheckTextColor(messageColor[i]));
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			chattingText.move(10, 650);
			window.draw(chattingText);
		}

		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		//text.move(10, 630);
		window.draw(text);

		if (myState == CharacterCreation)
		{
			for (int c = 0; c < differentClasses->size() + 1; c++)
			{
				std::string chatting = std::to_string(c + 1) + " - " + differentClasses[c];
				classesText.setFillColor(classesColor[c]);
				classesText.setPosition(sf::Vector2f(650, (50 * c) + 60));
				classesText.setString(chatting);
				window.draw(classesText);
			}
		}
		if (myState == CharacterCreation)
		{
			for (int c = 0; c < differentClasses->size() + 1; c++)
			{
				std::string chatting = std::to_string(c + 1) + " - " + differentClasses[c];
				classesText.setFillColor(classesColor[c]);
				classesText.setPosition(sf::Vector2f(650, (50 * c) + 60));
				classesText.setString(chatting);
				window.draw(classesText);
			}
		}
		
		//aqui van las imagenes
		mapaSprite.setPosition(mapaPosition);
		window.draw(mapaSprite);	

		for (int n = 0; n < aPlayers.size(); n++) {
			aPlayers[n].playerSprite.setTexture(aPlayers[n].playerTexture);
			aPlayers[n].playerSprite.setScale(0.25f, 0.25f);
			aPlayers[n].playerSprite.setPosition(aPlayers[n].position);
			window.draw(aPlayers[n].playerSprite);
		}
		myPlayer.playerSprite.setTexture(myPlayer.playerTexture);
		myPlayer.playerSprite.setScale(0.25f, 0.25f);
		myPlayer.playerSprite.setPosition(myPlayer.position);
		window.draw(myPlayer.playerSprite);	
		if (myState != CharacterCreation) {
			if (aPlayers.size() > 0) {
				for (int c = 0; c < aPlayers.size(); c++)
				{
					//std::string chatting = std::to_string(c + 1) + " - " + aPlayers[c].name + "-"+ aPlayers[c].clase.vida;
					std::string myName = std::to_string(aPlayers[c].clase.vida) + " - " + aPlayers[c].name;
					nameText.setFillColor(CheckTextColor(aPlayers[c].ID));
					nameText.setPosition(sf::Vector2f(aPlayers[c].x - 20, aPlayers[c].y - 20));
					nameText.setString(myName);
					window.draw(nameText);
				}
			}
			std::string myName = std::to_string(myPlayer.clase.vida) + " - " + myPlayer.name;
			nameText.setFillColor(CheckTextColor(myPlayer.ID));
			nameText.setPosition(sf::Vector2f(myPlayer.x - 20, myPlayer.y - 20));
			nameText.setString(myName);
			window.draw(nameText);
			
		}
		if (myState != CharacterCreation) {
			if (aPlayers.size() > 0) {
				for (int c = 0; c < aPlayers.size(); c++)
				{
					//std::string chatting = std::to_string(c + 1) + " - " + aPlayers[c].name + "-"+ aPlayers[c].clase.vida;
					std::string myName = std::to_string(aPlayers[c].clase.vida) + " - " + aPlayers[c].name;
					nameText.setFillColor(CheckTextColor(aPlayers[c].ID));
					nameText.setPosition(sf::Vector2f(aPlayers[c].x - 20, aPlayers[c].y - 20));
					nameText.setString(myName);
					window.draw(nameText);
				}
			}
			std::string myName = std::to_string(myPlayer.clase.vida) + " - " + myPlayer.name;
			nameText.setFillColor(CheckTextColor(myPlayer.ID));
			nameText.setPosition(sf::Vector2f(myPlayer.x - 20, myPlayer.y - 20));
			nameText.setString(myName);
			window.draw(nameText);
		}
		window.display();

		window.clear();
	}

	sockSelecThread.join();
}