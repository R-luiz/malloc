# Use Ubuntu as base image for C development
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    gdb \
    make \
    valgrind \
    vim \
    git \
    curl \
    wget \
    man-db \
    manpages-dev \
    libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /workspace

# Create a non-root user for development
RUN useradd -m -s /bin/bash developer && \
    chown -R developer:developer /workspace

# Switch to non-root user
USER developer

# Set default shell
SHELL ["/bin/bash", "-c"]

# Expose any ports you might need (optional)
# EXPOSE 8080

CMD ["/bin/bash"]
