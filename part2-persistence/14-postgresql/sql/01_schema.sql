-- Practice schema for chapter 14. Safe to run repeatedly.
DROP TABLE IF EXISTS loans, books, authors, members CASCADE;

CREATE TABLE authors (
    id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name       TEXT NOT NULL UNIQUE,
    country    TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE books (
    id        BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    author_id BIGINT NOT NULL REFERENCES authors(id) ON DELETE CASCADE,
    title     TEXT NOT NULL CHECK (length(title) > 0),
    year      INT CHECK (year BETWEEN 1400 AND 2100),
    price     NUMERIC(10, 2) NOT NULL DEFAULT 0 CHECK (price >= 0),
    copies    INT NOT NULL DEFAULT 1 CHECK (copies >= 0)
);
CREATE INDEX books_author_idx ON books(author_id);
CREATE INDEX books_title_idx ON books(lower(title));

CREATE TABLE members (
    id    BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    email TEXT NOT NULL UNIQUE,
    name  TEXT NOT NULL
);

CREATE TABLE loans (
    id          BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    book_id     BIGINT NOT NULL REFERENCES books(id),
    member_id   BIGINT NOT NULL REFERENCES members(id),
    loaned_at   TIMESTAMPTZ NOT NULL DEFAULT now(),
    returned_at TIMESTAMPTZ
);
