<?hh
function l(...) {
  $args = func_get_args();
  $output = array();
  ob_start();
  foreach ($args as $arg) {
    if (!(is_string($arg) || is_numeric($arg))) {
      var_dump($arg);
      $output[] = ob_get_contents();
    } else {
      $output[] = $arg;
    }
  }
  ob_end_clean();
  $message = implode(' ', $output) . PHP_EOL;
  $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
  $backtrace = array_shift($backtrace);

  logger($message, $backtrace['file'], $backtrace['line']);
}

function ls(...) {
  $args = func_get_args();
  $message = call_user_func_array('sprintf', $args);
  $backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
  $backtrace = array_shift($backtrace);

  logger($message, $backtrace['file'], $backtrace['line']);
}

function logger($message, $file = null, $line = null) {
  $resource = fopen($_ENV['BASE_LOG_FILE'], 'a+w');
  $log = sprintf('[%s:%d] %s',
    $file, $line, $message);

  fwrite($resource, $log);
  if (idx($_ENV, 'APPLICATION_ENV') !== 'prod' &&
    idx($_ENV, 'CHROME_LOGGING_ENABLED') &&
    idx($_ENV, 'WORKER_SCRIPT') == false) {
    ChromePhp::log($log);
  }
}

function fatal_log() {
  $errfile = "unknown file";
  $errstr = "shutdown";
  $errno = E_CORE_ERROR;
  $errline = 0;

  $error = error_get_last();

  if ($error !== NULL) {
    $errno = $error["type"];
    $errfile = $error["file"];
    $errline = $error["line"];
    $errstr = $error["message"];
  }

  switch ($errno) {
    case 16777217:
      $error_type = 'Fatal error';
      break;
    case E_NOTICE:
      $error_type = 'Notice';
      break;
    case 16: return;
    default:
      $error_type = 'Error ' . $errno;
  }

  $message = sprintf('%s: %s', $error_type, $errstr);
  logger($message, $errfile, $errline);
}

function idx($array, $key, $default = null) {
  if (is_array($array)) {
    return array_key_exists($key, $array) ? $array[$key] : $default;
  } elseif (is_object($array)) {
    return isset($array->$key) ? $array->$key : $default;
  } else {
    return $default;
  }
}

function mid($id = null) {
  return is_a($id, 'MongoId') ? $id : new MongoId($id);
}

function mdate($date = null) {
  $mdate = $date == null ? time() : strtotime($date);
  return is_a($date, 'MongoDate') ? $date : new MongoDate($mdate);
}

function sha256($data) {
  return hash('sha256', $data);
}

function s() {
  $args = func_get_args();
  return call_user_func_array('sprintf', $args);
}

function regex(string $pattern, string $subject) {
  $out = [];
  if (preg_match($pattern, $subject, $out) === false) {
    return null;
  }

  return $out;
}

function regex_all(string $pattern, string $subject, ?int &$matches = null) {
  $out = [];
  $m = preg_match_all($pattern, $subject, $out);
  if ($m === false) {
    return null;
  }

  $matches = $m;
  return $out;
}

function slugify($text) {
  // replace non letter or digits by -
  $text = preg_replace('~[^\pL\d]+~u', '-', $text);
  // transliterate
  $text = iconv('utf-8', 'us-ascii//TRANSLIT', $text);
  // remove unwanted characters
  $text = preg_replace('~[^-\w]+~', '', $text);
  // trim
  $text = trim($text, '-');
  // remove duplicate -
  $text = preg_replace('~-+~', '-', $text);
  // lowercase
  $text = strtolower($text);
  if (empty($text)) {
    return 'n-a';
  }
  return $text;
}

function recursive_array_diff($a1, $a2) {
  $r = array();
  foreach ($a1 as $k => $v) {
    if (array_key_exists($k, $a2)) {
      if (is_array($v)) {
        $rad = recursive_array_diff($v, $a2[$k]);
        if (count($rad)) {
          $r[$k] = $rad;
        }
      } else {
        if ($v != $a2[$k]) {
          $r[$k] = $v;
        }
      }
    } else {
      $r[$k] = $v;
    }
  }
  return $r;
}
