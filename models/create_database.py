import sqlite3
conn = sqlite3.connect('example.db')  # You can create a new database by changing the name within the quotes
c = conn.cursor() # The database will be saved in the location where your 'py' file is saved

# Create table - CLIENTS
c.execute('''CREATE TABLE IF NOT EXISTS EBAY_RESPONSE (
          ID INTEGER PRIMARY KEY,
          USERNAME TEXT NOT NULL UNIQUE,
          BODY           TEXT    NOT NULL,
          TIMESTAMP INTEGER  NOT NULL);''')

c.execute('''CREATE UNIQUE INDEX IF NOT EXISTS index_name on EBAY_RESPONSE (USERNAME);''')
          
               
conn.commit()