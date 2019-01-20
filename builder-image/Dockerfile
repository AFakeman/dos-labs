FROM suchja/wine


USER root
COPY waitonprocess.sh /scripts/
RUN chmod +x /scripts/waitonprocess.sh

USER xclient
RUN wine wineboot --init \
&& /scripts/waitonprocess.sh wineserver

COPY WATCOM /home/xclient/.wine/drive_c/WATCOM
