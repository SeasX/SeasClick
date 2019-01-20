FROM php:7.2-fpm
ADD ./ /data/SeasClick/
RUN ( cd /data/SeasClick \
        && phpize \
        && ./configure \
        && make -j "$(nproc)" \
        && make install \
    ) \
    && docker-php-ext-enable SeasClick
