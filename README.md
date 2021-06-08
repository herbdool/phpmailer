PHPMailer
=========

Adds SMTP support for sending e-mails using the PHPMailer library.

Requirements
------------

* Access to an SMTP server

Optional:

* To connect to an SMTP server using SSL, your PHP installation needs to have
  OpenSSL support.

* Mime Mail module to send HTML e-mails
  https://backdropcms.org/project/mimemail

Installation
------------

* Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules


Configuration
-------------

* Configure user permissions at Administer >> User management >> Access
  control >> phpmailer module.

  Only users with the "administer phpmailer settings" permission are allowed to
  access the module configuration page.

* Configure your SMTP server settings at Administer >> Site
  configuration >> PHPMailer.

  If used in conjunction with Mime Mail, PHPMailer will show up as an e-mail
  engine in the Mime Mail module settings.  To let Mime Mail apply styles and
  formatting to system e-mails but still use PHPMailer for mail transport,
  uncheck "Use PHPMailer to send e-mails" first on the PHPMailer settings
  page.  Then activate "Use mime mail for all messages" on the Mime Mail
  settings page and choose PHPMailer from the list of available e-mail engines.

* Using Google Mail or Google Apps as SMTP server

  To send e-mails with Google Mail use the following settings:

    SMTP server:     smtp.gmail.com
    SMTP port:       465
    Secure protocol: SSL
    Username:        your_google_mail_name@gmail.com
      or:            your_username@your_google_apps_domain.com
    Password:        your_google_mail_password

  In Advanced SMTP settings:
    Enable 'Always set "Reply-To" address' checkbox.

  Also note the sending limits for Google Mail accounts:
  http://mail.google.com/support/bin/answer.py?hl=en&answer=22839

  General instructions for Google Mail accounts:
  http://mail.google.com/support/bin/answer.py?answer=13287

* Using other popular freemailers as SMTP server

  See http://phpmailer.worxware.com/index.php?pg=tip_srvrs.

* Errors when trying to send a test email

  If you see the following error messages when trying to send a test email from
  the settings page:

    SMTP -> ERROR: Failed to connect to server: Connection timed out (110)
    SMTP Error: Could not connect to SMTP host. 

  it means the mail server can not be reached, usually because your hosting
  provider is blocking the port by a firewall. The solution is to ask your
  provider to allow outgoing connections to your mail server.

* Debug settings

  PHPMailer supports rerouting all e-mails for debugging purposes, to prevent
  you from accidentally sending out e-mails to real users from a development
  site.  To enable this feature, add the following lines to the end of your
  settings.php (usually located in sites/default):

    `$settings['phpmailer_debug_email'] = 'your_debug_email@yoursite.com';`

  This will change the recipient of all e-mails to the configured address.

Issues
------

Bugs and Feature requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/phpmailer/issues

Current Maintainers
-------------------

- [Jason Flatt (oadaeh)](https://github.com/oadaeh)
- You?

Credits
-------

- Ported to Backdrop by Herb v/d Dool (https://github.com/herbdool/)
- Authors: Stefan M. Kudwien (smk-ka) - http://drupal.org/user/48898 and Daniel F. Kudwien (sun) - http://drupal.org/user/54136

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
