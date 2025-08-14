Recommended services:

- Python 3.13
  - FastAPI
  - SQLModel
- Docker with Docker Compose
- PostgreSQL 17
- Redis

Write automated tests for all services using pytest.
https://fastapi.tiangolo.com/tutorial/testing/

Place new routes into a `routes` directory. Use concise and clear naming conventions for routes.

Include health checks for each service in the Docker Compose file. 
Always use `docker compose` instead of `docker-compose` for compatibility with the latest Docker versions.

Try to use cli tools for setting up things 