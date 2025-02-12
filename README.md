# Book Management Application

## Overview
The **Book Management Application** is a simple desktop application that allows users to:
- Search for books using the **Open Library API**
- View book details (title, author, publication year, etc.)
- Save favorite books
- Leave personal notes on favorite books
- Save the list of favorite books with notes to a file

## Features
- **Book Search:** Enter a query and retrieve book details.
- **Favorites List:** Save books to a favorites list.
- **Personal Notes:** Add and store notes for each favorite book.
- **Export Favorites:** Save favorite books and notes to a text file.
- **Multithreading Support:** Uses `std::jthread` for efficient data fetching.

## Technologies Used
- **C++ 17/20**
- **ImGui** (for GUI rendering)
- **httplib** (for HTTP requests)
- **nlohmann::json** (for JSON parsing)
- **std::thread & std::mutex** (for multithreading)

## Installation
1. Clone the repository:
   ```sh
   git clone https://github.com/anasshwi/book-management-app.git
   cd book-management-app
   ```
2. Install Microsoft Visual Studio
3. Open up the file named as: "BookManagerApp.sln"
4. Run the Debugging tool to Test the App
   
## Fixing Errors
- If you face errors like missing .dll files (libcrypto.. / libssl...)
- You can find these files in the following path: "\shared\HttpSrc\openssl\bin", copy them next to the .exe file of the app "${projectDirectory}\x64\".
- Or you can install OpenSSL from Shining Light Productions.

## Usage
1. Launch the application.
2. Use the search bar to find books.
3. Click on a book to view details and to add it to favorites.
4. Check the box on the top of the window to view Favorites List Window.
5. Click "Leave Note" to add a personal note.
6. Click "Save Favorites to File" to export favorites.

