INSERT INTO authors(name, country) VALUES
    ('Bjarne Stroustrup', 'Denmark'),
    ('Scott Meyers', 'USA'),
    ('Anthony Williams', 'UK'),
    ('Robert Nystrom', 'USA');

INSERT INTO books(author_id, title, year, price, copies)
SELECT a.id, b.title, b.year, b.price, b.copies
FROM (VALUES
    ('Bjarne Stroustrup', 'A Tour of C++', 2022, 39.99, 3),
    ('Bjarne Stroustrup', 'The C++ Programming Language', 2013, 69.99, 1),
    ('Bjarne Stroustrup', 'Programming: Principles and Practice Using C++', 2024, 59.99, 2),
    ('Scott Meyers', 'Effective Modern C++', 2014, 44.99, 4),
    ('Scott Meyers', 'Effective STL', 2001, 29.99, 0),
    ('Anthony Williams', 'C++ Concurrency in Action', 2019, 49.99, 2),
    ('Robert Nystrom', 'Game Programming Patterns', 2014, 0.00, 5)
) AS b(author, title, year, price, copies)
JOIN authors a ON a.name = b.author;

INSERT INTO members(email, name) VALUES
    ('ada@example.com', 'Ada'), ('grace@example.com', 'Grace'), ('linus@example.com', 'Linus');

INSERT INTO loans(book_id, member_id)
SELECT b.id, m.id FROM books b, members m
WHERE (b.title, m.name) IN (('A Tour of C++', 'Ada'), ('Effective Modern C++', 'Grace'), ('A Tour of C++', 'Linus'));
