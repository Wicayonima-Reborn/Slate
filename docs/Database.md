# Database Schema

## SQLite Database
Primary local storage, slate.db.

## Tables

### files
| Column   | Type    | Description |
|----------|---------|-------------|
| id       | INTEGER | Primary key |
| path     | TEXT    | Full file path (unique) |
| language | TEXT    | Detected language |

### symbols
| Column   | Type    | Description |
|----------|---------|-------------|
| id       | INTEGER | Primary key |
| name     | TEXT    | Symbol name |
| kind     | TEXT    | function, class, struct, macro |
| file_id  | INTEGER | Foreign key (files.id) |
| line     | INTEGER | Line number |
| column   | INTEGER | Column number |

### dependencies
| Column            | Type    | Description |
|-------------------|---------|-------------|
| id                | INTEGER | Primary key |
| source_symbol_id  | INTEGER | FK (symbols.id) |
| target_symbol_id  | INTEGER | FK (symbols.id) |
| kind              | TEXT    | include, call, inherit |

### indexing_sessions
| Column      | Type    | Description |
|-------------|---------|-------------|
| id          | INTEGER | Primary key |
| started_at  | TEXT    | Datetime |
| finished_at | TEXT    | Datetime |
| status      | TEXT    | running, completed |

## Migrations
All schema changes are applied in Database::migrate(). Tables are created with IF NOT EXISTS to ensure idempotency.
