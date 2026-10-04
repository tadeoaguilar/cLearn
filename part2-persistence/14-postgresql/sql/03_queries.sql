-- Practice queries. Run them one at a time in psql and read the results.

-- 1. All books with their author, newest first
SELECT b.title, a.name AS author, b.year
FROM books b JOIN authors a ON a.id = b.author_id
ORDER BY b.year DESC;

-- 2. Number of books and average price per author (LEFT JOIN keeps authors with 0 books)
SELECT a.name, count(b.id) AS books, round(avg(b.price), 2) AS avg_price
FROM authors a LEFT JOIN books b ON b.author_id = a.id
GROUP BY a.name ORDER BY books DESC;

-- 3. Books currently on loan, and who has them
SELECT b.title, m.name, l.loaned_at::date
FROM loans l JOIN books b ON b.id = l.book_id JOIN members m ON m.id = l.member_id
WHERE l.returned_at IS NULL;

-- 4. Available copies = copies - active loans
SELECT b.title, b.copies - count(l.id) FILTER (WHERE l.returned_at IS NULL) AS available
FROM books b LEFT JOIN loans l ON l.book_id = b.id
GROUP BY b.id ORDER BY available;

-- 5. Case-insensitive search that can use the lower(title) index
EXPLAIN ANALYZE SELECT * FROM books WHERE lower(title) LIKE lower('effective%');

-- 6. Upsert: insert or update on conflict
INSERT INTO members(email, name) VALUES ('ada@example.com', 'Ada Lovelace')
ON CONFLICT (email) DO UPDATE SET name = EXCLUDED.name
RETURNING id, name;

-- 7. A transaction you can try interactively (then ROLLBACK instead of COMMIT)
BEGIN;
UPDATE books SET copies = copies - 1 WHERE title = 'A Tour of C++';
SELECT title, copies FROM books WHERE title = 'A Tour of C++';
ROLLBACK;
