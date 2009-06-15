/* $Id$ */

-- SUMMARY --

Adds SMTP support for sending e-mails using the PHPMailer library.

For a full description of the module, visit the project page:
  http://drupal.org/project/phpmailer
To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/phpmailer


-- REQUIREMENTS --

* Access to an SMTP server

* PHPMailer for PHP5/6
  http://phpmailer.codeworxtech.com

Optional:

* To connect to an SMTP server using SSL, your PHP installation needs to have
  OpenSSL support.

* Mime Mail module to send HTML e-mails
  http://drupal.org/project/mimemail

* Personalized E-mails module to adjust the displayed sender name
  http://drupal.org/project/pmail


-- INSTALLATION --

1. Download PHPMailer for PHP5/6 from

     http://sourceforge.net/project/showfiles.php?group_id=26031&package_id=252700

   and extract the the following files to the subdirectory 'phpmailer' of this
   module:

     class.phpmailer.php
     class.smtp.php
 
   Be careful NOT to extract the path names contained in the archive.

2. Install as usual, see http://drupal.org/node/70151 for further information.


-- CONFIGURATION --

* Configure user permissions at Administer >> User management >> Access control
  >> phpmailer module.

  Only users with the "administer phpmailer settings" permission will gain
  access to the module configuration page.

* When not using the optional Mime Mail module, customize module settings at
  Administer >> Site configuration >> PHPMailer.

  If used in conjunction with Mime Mail, PHPMailer will show up as an e-mail
  engine in the Mime Mail module settings.  Note that there will be no separate
  PHPMailer configuration page in this case.

* Using Google Mail as SMTP server

  To send e-mails with Google Mail use the following settings:

    SMTP server:     smtp.gmail.com
    SMTP port:       465
    Secure protocol: SSL
    Username:        your_google_mail_name@gmail.com
    Password:        your_google_mail_password

  Note however, that Google automatically rewrites the "from" line of any e-mail
  you send via their SMTP gateway to your Google Mail address.

* Debug settings

  PHPMailer supports rerouting all e-mails for debugging purposes, so you don't
  accidentally send e-mails to your users from a development site.  To enable
  this feature, add the following lines to the end of your settings.php (usually
  located in sites/default):

    $conf = array(
      'phpmailer_debug_email' => 'your_debug_email@yoursite.com',
    );

  This will change the recipient of all e-mails to the configured address.


-- CREDITS --

Authors:
* Stefan M. Kudwien (smk-ka) - http://drupal.org/user/48898
* Daniel F. Kudwien (sun) - http://drupal.org/user/54136

This project has been sponsored by UNLEASHED MIND
Specialized in consulting and planning of Drupal powered sites, UNLEASHED
MIND offers installation, development, theming, customization, and hosting
to get you started. Visit http://www.unleashedmind.com for more information.

