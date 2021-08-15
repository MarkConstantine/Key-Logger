using klog_listener.Entities;
using Microsoft.EntityFrameworkCore;

namespace klog_listener.Contexts
{
    public class KeyLogContext : DbContext
    {
        public KeyLogContext(DbContextOptions<KeyLogContext> options) : base(options)
        {

        }

        public DbSet<KeyLog> KeyLogs { get; set; } 
    }
}
