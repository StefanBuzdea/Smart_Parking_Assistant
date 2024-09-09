# Smart Parking Assistant

## Project Overview
The **Smart Parking Assistant** is a client/server application designed to assist drivers in finding available parking spaces in real-time. By collecting data from parking sensors, the system updates the parking map dynamically, allowing drivers to quickly find and occupy available spots. The application helps reduce the stress and time spent searching for parking spaces, contributing to a more efficient and optimized parking experience.

This project focuses on a single parking area but can be scaled to cover multiple parking lots.

## Key Features
- **Real-time Parking Updates**: The system uses sensors to monitor parking spots, updating the parking map based on spot availability.
- **Driver Assistance**: Drivers can view available and occupied spots on a map, allowing them to navigate directly to free spaces.
- **Automatic Parking Updates**: Once a driver occupies a spot, the sensor updates the map for all users in real time, without further interaction from the driver.
- **Scalability**: Though currently developed for a single parking lot, the system can be extended to manage multiple parking areas.

## Technology Stack
The application uses a **client-server architecture** and communicates through **TCP (Transmission Control Protocol)** for reliable data transfer. Key technologies involved include:
- **Server-Client Communication**: Each client (driver) is managed by an individual thread, ensuring that the server handles all active users efficiently.
- **TCP Protocol**: Chosen for its reliability in delivering data in the correct order, preventing confusion caused by lost or out-of-sequence messages.
- **Non-blocking I/O**: Both clients and sensors use non-blocking communication to ensure smooth real-time updates without performance degradation.

## How It Works
1. **Driver Connects**: Upon entering the parking lot, a driver connects to the application and receives a map showing all available and occupied parking spots.
2. **Sensor Updates**: Each parking spot is equipped with a sensor that sends occupancy data to the server. When a spot is taken, the sensor informs the server, and the parking map is updated for all users.
3. **Real-time Updates**: The system automatically refreshes the map as soon as changes occur, ensuring drivers always have accurate parking information.

## System Components
- **Server**: Manages communication between sensors and drivers. Updates and distributes the parking map to all connected clients.
- **Sensors**: Installed at each parking spot to detect availability and communicate changes to the server.
- **Clients**: Drivers who connect to the system to view the parking map and find available spots.

## Future Enhancements
- **GUI**: A graphical user interface that visually represents the parking lot with color-coded spots (green for available spaces, red for occupied spaces) to make it easier for drivers to find parking.
- **Parking Spot Reservations**: Implement a feature that allows drivers to reserve parking spots in advance, ensuring they have a place when they arrive.
- **Parking History**: Store and display driver details and parking history, allowing for future use and better monitoring of parking habits.
- **License Plate Recognition**: Integrate cameras that automatically detect and log vehicles' license plates as they enter the parking lot, streamlining the parking process and providing additional security.

## Authors
- **Ștefan Buzdea**  
  Faculty of Computer Science  
  Alexandru Ioan Cuza University, Iași, Romania
