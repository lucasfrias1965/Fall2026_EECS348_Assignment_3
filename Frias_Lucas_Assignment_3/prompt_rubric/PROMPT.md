Make this program in C++. You have the added requirement to code using objects, not functions.

•	The program will prioritize emails for a busy company CEO.  
•	You will use a MaxHeap as a means of implementing a priority queue. A priority queue is a queue where emails can shift towards the front of the queue based on a priority status. 
•	You must implement a MaxHeap using a list-based implementation. Then use that MaxHeap to handle all your email prioritizing for the CEO. 
•	You must create functions from scratch. Do not include pre-existing heap modules. 


Here's the file format:

```
-------------
EMAIL <sender category>, <subject line>, <date> - The emails in the CEO’s Inbox should be placed in queue based on their sender category and date. The sender categories and priority to be read are as follows:  
     •	Boss – read first   
     •	Subordinate – read next  
     •	Peer – read next   
     •	ImportantPerson – read next   
     •	OtherPerson – read last   
     If there is more than one from a sender, then the newest email (not the oldest) should be read first. I discovered this trick while a manager at Sprint.      EMAIL is followed by space. The rest of the fields are delimited.   Assume <sender category> is one of the five strings listed above.   Assume <subject line> is a string which may contain spaces, but not commas   Assume <date> is in the format: MM-DD-YYYY
--------------
NEXT - Next email for the CEO to read. Display the information on the terminal in the following format: 
     Sender: 
     Subject: 
     Date: 
------------
READ - CEO has read with the email and has dealt with it
-----------
COUNT - display current count of untracked files

```

Here's an example file:
```
EMAIL Peer,Can you help me on this?,12-01-2024 
EMAIL OtherPerson,Try our product,12-19-2024 
EMAIL Boss,Important,12-20-2024 
EMAIL Subordinate,How do I handle this?,12-25-2024 
EMAIL ImportantPerson,Health Insurance Enrollment,12-31-2024 
EMAIL Boss,Never Mind,01-03-2025 
COUNT 
NEXT 
READ 
NEXT 
READ 
COUNT 
```
Here is the output that the program should give
```
There are 6 emails to read. 
 
Next email: 
Sender: Boss 
Subject: Never Mind 
Date: 01-03-2025 
 
Next email: 
Sender: Boss 
Subject: Important 
Date: 12-20-2024 
 
There are 4 emails to read. 
```

Good luck!
